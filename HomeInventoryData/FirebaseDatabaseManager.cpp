#include "firebasedatabasemanager.h"
#include "HomeObject.h"
#include <QNetworkRequest>
#include <QJsonDocument>
#include <QJsonObject>
#include <QJsonArray>
#include <QUrlQuery>
#include <QDebug>
#include <iostream>
#include <QDateTime>

FirebaseDatabaseManager::FirebaseDatabaseManager(QObject* parent)
    : QObject(parent)
    , m_isConnected(false)
	, m_isAuthenticated(false)
    , m_networkManager(new QNetworkAccessManager(this))
	, m_credentialsManager(new CredentialsManager())
    , m_isRefreshingToken(false)
{
}

FirebaseDatabaseManager::~FirebaseDatabaseManager()
{
    disconnect();
    delete m_credentialsManager;
}

bool FirebaseDatabaseManager::connect(const QString& firebaseUrl)
{
    m_firebaseUrl = firebaseUrl;

    // Rimuovi trailing slash se presente
    if (m_firebaseUrl.endsWith('/')) {
        m_firebaseUrl.chop(1);
    }

	m_isConnected = true; //(Checkare perchè mi sembra brutto metterlo a true senza/prima di testare la connessione)

    std::cout << u8"🔗 Firebase URL configured: ";
    qDebug() << m_firebaseUrl;

    return true;
    // Test della connessione con una semplice GET request
    //QJsonDocument testDoc = performGetRequest("/.json");

    //if (!testDoc.isNull()) {
    //    m_isConnected = true;
    //    setLastError(QString());
    //    std::cout << u8"✅ Connected to Firebase: ";
    //    qDebug() << m_firebaseUrl;
    //    return true;
    //}

    //m_isConnected = false;
    //setLastError("Failed to connect to Firebase database");
    //std::cout << u8"❌ Failed to connect to Firebase: ";
    //qDebug() << m_firebaseUrl;
    //return false;
}

bool FirebaseDatabaseManager::setApiKey(const QString& apiKey)
{
    if (apiKey.isEmpty()) {
        setLastError("API Key cannot be empty");
        return false;
    }

    m_apiKey = apiKey;
    std::cout << u8"🔑 API Key configured\n";
    return true;
}

bool FirebaseDatabaseManager::tryAutoLogin()
{
    if (!m_credentialsManager->hasStoredCredentials()) {
        std::cout << u8"ℹ️ No saved credentials found\n";
        return false;
    }

    std::cout << u8"🔐 Attempting auto-login with saved credentials...\n";

    QString email, password;
    if (!m_credentialsManager->loadCredentials(email, password)) {
        setLastError("Failed to load saved credentials");
        return false;
    }

    // Prova prima con il refresh token se disponibile
    QString refreshToken = m_credentialsManager->loadRefreshToken();
    if (!refreshToken.isEmpty()) {
        m_refreshToken = refreshToken;
        if (refreshAccessToken()) {
            m_userEmail = email;
            m_isAuthenticated = true;
            std::cout << u8"✅ Auto-login successful with refresh token!\n";
            std::cout << u8"👤 Logged in as: ";
            qDebug() << m_userEmail;
            emit authenticationCompleted(true, m_userEmail);
            return true;
        }
    }

    // Altrimenti usa email e password
    bool success = signInWithEmailPassword(email, password);
    if (success) {
        std::cout << u8"✅ Auto-login successful!\n";
        std::cout << u8"👤 Logged in as: ";
        qDebug() << m_userEmail;
        emit authenticationCompleted(true, m_userEmail);
    }

    return success;
}

bool FirebaseDatabaseManager::authenticateWithEmail(const QString& email, const QString& password, bool rememberMe)
{
    if (email.isEmpty() || password.isEmpty()) {
        setLastError("Email and password cannot be empty");
        return false;
    }

    std::cout << u8"🔐 Authenticating with email/password...\n";

    bool success = signInWithEmailPassword(email, password);

    if (success) {
        std::cout << u8"✅ Authentication successful!\n";
        std::cout << u8"👤 Logged in as: ";
        qDebug() << m_userEmail;

        // Salva le credenziali se richiesto
        if (rememberMe) {
            if (m_credentialsManager->saveCredentials(email, password)) {
                std::cout << u8"💾 Credentials saved for auto-login\n";
            }
            if (!m_refreshToken.isEmpty()) {
                m_credentialsManager->saveRefreshToken(m_refreshToken);
            }
        }

        emit authenticationCompleted(true, m_userEmail);
    }
    else {
        std::cout << u8"❌ Authentication failed!\n";
        emit authenticationCompleted(false, "");
    }

    return success;
}

bool FirebaseDatabaseManager::signInWithEmailPassword(const QString& email, const QString& password)
{
    if (m_apiKey.isEmpty()) {
        setLastError("API Key not configured. Call setApiKey() first.");
        return false;
    }

    QUrl url("https://identitytoolkit.googleapis.com/v1/accounts:signInWithPassword");
    QUrlQuery query;
    query.addQueryItem("key", m_apiKey);
    url.setQuery(query);

    QNetworkRequest request(url);
    request.setHeader(QNetworkRequest::ContentTypeHeader, "application/json");

    QJsonObject postData;
    postData["email"] = email;
    postData["password"] = password;
    postData["returnSecureToken"] = true;

    QJsonDocument doc(postData);
    QNetworkReply* reply = m_networkManager->post(request, doc.toJson());

    QEventLoop loop;
    QTimer timer;
    timer.setSingleShot(true);

    QObject::connect(reply, &QNetworkReply::finished, &loop, &QEventLoop::quit);
    QObject::connect(&timer, &QTimer::timeout, &loop, &QEventLoop::quit);

    timer.start(10000);
    loop.exec();

    if (!timer.isActive()) {
        setLastError("Request timeout");
        reply->deleteLater();
        return false;
    }
    timer.stop();

    bool success = false;

    if (reply->error() == QNetworkReply::NoError) {
        QByteArray response = reply->readAll();
        QJsonDocument responseDoc = QJsonDocument::fromJson(response);
        QJsonObject responseObj = responseDoc.object();

        m_idToken = responseObj["idToken"].toString();
        m_refreshToken = responseObj["refreshToken"].toString();
        m_userEmail = responseObj["email"].toString();
        m_userId = responseObj["localId"].toString();

        int expiresIn = responseObj["expiresIn"].toString().toInt();
        if (expiresIn == 0) expiresIn = 3600; // Default 1 hr
		m_tokenExpiry = QDateTime::currentDateTime().addSecs(expiresIn - 300); // Refresh 5 min before expiry

        m_isAuthenticated = !m_idToken.isEmpty();
        success = m_isAuthenticated;
    }
    else {
        QByteArray response = reply->readAll();
        QJsonDocument responseDoc = QJsonDocument::fromJson(response);
        QJsonObject errorObj = responseDoc.object()["error"].toObject();
        QString errorMessage = errorObj["message"].toString();

        // Messaggi di errore user-friendly
        if (errorMessage.contains("INVALID_PASSWORD")) {
            setLastError("Invalid password");
        }
        else if (errorMessage.contains("EMAIL_NOT_FOUND")) {
            setLastError("Email not found");
        }
        else if (errorMessage.contains("USER_DISABLED")) {
            setLastError("This account has been disabled");
        }
        else if (errorMessage.contains("TOO_MANY_ATTEMPTS")) {
            setLastError("Too many failed attempts. Please try again later");
        }
        else {
            setLastError("Authentication failed: " + errorMessage);
        }
    }

    reply->deleteLater();
    return success;
}

bool FirebaseDatabaseManager::refreshAccessToken()
{
    if (m_isRefreshingToken) {
        std::cout << u8"⏳ Token refresh already in progress, skipping...\n";
        return false;
    }

    if (m_refreshToken.isEmpty()) {
        setLastError("No refresh token available");
        return false;
    }

    if (m_apiKey.isEmpty()) {
        setLastError("API Key not configured");
        return false;
    }

    std::cout << u8"🔄 Starting token refresh...\n";
	m_isRefreshingToken = true;

    QUrl url("https://securetoken.googleapis.com/v1/token");
    QUrlQuery query;
    query.addQueryItem("key", m_apiKey);
    url.setQuery(query);

    QNetworkRequest request(url);
    request.setHeader(QNetworkRequest::ContentTypeHeader, "application/x-www-form-urlencoded");

    QString postData = QString("grant_type=refresh_token&refresh_token=%1").arg(m_refreshToken);

    QNetworkReply* reply = m_networkManager->post(request, postData.toUtf8());

    QEventLoop loop;
    QTimer timer;
    timer.setSingleShot(true);

    QObject::connect(reply, &QNetworkReply::finished, &loop, &QEventLoop::quit);
    QObject::connect(&timer, &QTimer::timeout, &loop, &QEventLoop::quit);

    timer.start(10000);
    loop.exec();

    if (!timer.isActive()) {
        setLastError("Token refresh timeout");
        reply->deleteLater();
		m_isRefreshingToken = false;
        return false;
    }
    timer.stop();

    bool success = false;

    if (reply->error() == QNetworkReply::NoError) {
        QByteArray responseData = reply->readAll();
        // std::cout << u8"📥 Refresh response:"; qDebug() << responseData;  // DUBUG: print all json response data, very long, decomment only if needed

        QJsonDocument responseDoc = QJsonDocument::fromJson(responseData);
        QJsonObject responseObj = responseDoc.object();

        QString newIdToken = responseObj["id_token"].toString();
        QString newRefreshToken = responseObj["refresh_token"].toString();

        if (!newIdToken.isEmpty()) {
            m_idToken = newIdToken;
            m_refreshToken = newRefreshToken;

            // Aggiorna la scadenza del token
            int expiresIn = responseObj["expires_in"].toString().toInt();
            if (expiresIn == 0) expiresIn = 3600;
            m_tokenExpiry = QDateTime::currentDateTime().addSecs(expiresIn - 300);

            success = true;

            // Salva il nuovo refresh token
            m_credentialsManager->saveRefreshToken(m_refreshToken);

            std::cout << u8"\n✅ Token refreshed successfully";
            std::cout << u8"\n🔑 New token length:" << m_idToken.length();
            std::cout << u8"\n⏰ Token expires:";
            qDebug() << m_tokenExpiry.toString();
        }
        else {
            setLastError("Token refresh returned empty token");
            std::cout << u8"\n❌ Empty token in response";
        }
    }
    else {
        QByteArray errorData = reply->readAll();
        std::cout << u8"\n❌ Refresh error:" << errorData;
        setLastError("Token refresh failed: " + reply->errorString());
    }

    reply->deleteLater();
	m_isRefreshingToken = false;
    return success;
}

void FirebaseDatabaseManager::disconnect()
{
    m_isConnected = false;
    m_isAuthenticated = false;
    m_firebaseUrl.clear();
    m_idToken.clear();
    m_refreshToken.clear();
    m_userEmail.clear();
    m_userId.clear();
}

void FirebaseDatabaseManager::logout(bool clearSavedCredentials)
{
    std::cout << u8"👋 Logging out user: ";
    qDebug() << m_userEmail;

    if (clearSavedCredentials) {
        m_credentialsManager->clearCredentials();
        std::cout << u8"🗑️ Saved credentials cleared\n";
    }

    disconnect();
}

bool FirebaseDatabaseManager::isConnected() const
{
    return m_isConnected;
}

bool FirebaseDatabaseManager::isAuthenticated() const
{
    return m_isAuthenticated;
}

QString FirebaseDatabaseManager::currentUserEmail() const
{
    return m_userEmail;
}

QString FirebaseDatabaseManager::lastError() const
{
    return m_lastError;
}

void FirebaseDatabaseManager::setLastError(const QString& error)
{
    m_lastError = error;
    if (!error.isEmpty()) {
        std::cout << u8"❌ Firebase Error: ";
        qDebug() << error;
    }
}

QString FirebaseDatabaseManager::buildUrl(const QString& path) const
{
    QString url = m_firebaseUrl + path;

    if (m_isAuthenticated && !m_idToken.isEmpty()) {
        QUrl qUrl(url);
        QUrlQuery query(qUrl);
        query.addQueryItem("auth", m_idToken);
        qUrl.setQuery(query);

        // DEBUG: Stampa l'URL (solo i primi caratteri del token per sicurezza)
        std::cout << u8"🔗 Request URL: "; 
        qDebug() << qUrl.toString().left(100) + "...";
        std::cout << u8"🔑 Token present:" << !m_idToken.isEmpty() << "Length:" << m_idToken.length() << "\n";

        return qUrl.toString();
    }

    std::cout << u8"⚠️ Building URL without auth token!\n";
    return url;
}

// ==================== GET REQUEST ====================
QJsonDocument FirebaseDatabaseManager::performGetRequest(const QString& path, int retryCount)
{
    if(!m_isAuthenticated) {
        setLastError("Not authenticated. Please log in first.");
        emit authenticationRequired();
        return QJsonDocument();
	}

    QUrl url(buildUrl(path));
    QNetworkRequest request(url);
    request.setHeader(QNetworkRequest::ContentTypeHeader, "application/json");

    QNetworkReply* reply = m_networkManager->get(request);

    // Attendi la risposta (modo sincrono per semplicità)
    QEventLoop loop;
    QTimer timer;
    timer.setSingleShot(true);

    QObject::connect(reply, &QNetworkReply::finished, &loop, &QEventLoop::quit);
    QObject::connect(&timer, &QTimer::timeout, &loop, &QEventLoop::quit);

    timer.start(10000); // 10 secondi timeout
    loop.exec();

    if (!timer.isActive()) {
        setLastError("Request timeout");
        reply->deleteLater();
        return QJsonDocument();
    }
    timer.stop();

    if (reply->error() == QNetworkReply::AuthenticationRequiredError) {
        reply->deleteLater();

        if (retryCount < 1) {  // <-- CAMBIATO: solo 1 retry
            std::cout << u8"⚠️ Authentication error, attempting token refresh...\n";
            if (refreshAccessToken()) {
                std::cout << u8"🔄 Retrying request after token refresh...\n";
                return performGetRequest(path, retryCount + 1);  // <-- Incrementa retry
            }
        }

        setLastError("Authentication expired. Please login again.");
        m_isAuthenticated = false; 
        emit authenticationRequired();
        return QJsonDocument();
    }

    if (reply->error() != QNetworkReply::NoError) {
        setLastError(reply->errorString());
        reply->deleteLater();
        return QJsonDocument();
    }

    QByteArray responseData = reply->readAll();
    reply->deleteLater();

    QJsonParseError parseError;
    QJsonDocument doc = QJsonDocument::fromJson(responseData, &parseError);

    if (parseError.error != QJsonParseError::NoError) {
        setLastError("JSON parse error: " + parseError.errorString());
        return QJsonDocument();
    }

    return doc;
}

// ==================== PUT REQUEST ====================
bool FirebaseDatabaseManager::performPutRequest(const QString& path, const QJsonValue& data, int retryCount)
{
    if (!m_isAuthenticated) {
        setLastError("Not authenticated. Please log in first.");
        emit authenticationRequired();
        return false;
    }

    QUrl url(buildUrl(path));
    QNetworkRequest request(url);
    request.setHeader(QNetworkRequest::ContentTypeHeader, "application/json");

    QJsonDocument doc;
    if (data.isObject()) {
        doc = QJsonDocument(data.toObject());
    }
    else if (data.isArray()) {
        doc = QJsonDocument(data.toArray());
    }

    QByteArray jsonData = doc.toJson(QJsonDocument::Compact);
    QNetworkReply* reply = m_networkManager->put(request, jsonData);

    QEventLoop loop;
    QTimer timer;
    timer.setSingleShot(true);

    QObject::connect(reply, &QNetworkReply::finished, &loop, &QEventLoop::quit);
    QObject::connect(&timer, &QTimer::timeout, &loop, &QEventLoop::quit);

    timer.start(10000);
    loop.exec();

    if (!timer.isActive()) {
        setLastError("Request timeout");
        reply->deleteLater();
        return false;
    }
    timer.stop();

    if (reply->error() == QNetworkReply::AuthenticationRequiredError) {
        reply->deleteLater();

        if (retryCount < 1) {  // <-- Solo 1 retry
            std::cout << u8"\n⚠️ Authentication error, attempting token refresh...";
            if (refreshAccessToken()) {
                std::cout << u8"\n🔄 Retrying request after token refresh...";
                return performPutRequest(path, data, retryCount + 1);
            }
        }

        setLastError("Authentication expired. Please login again.");
        m_isAuthenticated = false;
        emit authenticationRequired();
        return false;
    }

    bool success = (reply->error() == QNetworkReply::NoError);
    if (!success) {
        setLastError(reply->errorString());
    }
    else {
        setLastError(QString());
    }

    reply->deleteLater();
    return success;
}

// ==================== POST REQUEST ====================
bool FirebaseDatabaseManager::performPostRequest(const QString& path, const QJsonValue& data, int retryCount)
{
    if (!m_isAuthenticated) {
        setLastError("Not authenticated. Please login first.");
        emit authenticationRequired();
        return false;
    }

    QUrl url(buildUrl(path));
    QNetworkRequest request(url);
    request.setHeader(QNetworkRequest::ContentTypeHeader, "application/json");

    QJsonDocument doc;
    if (data.isObject()) {
        doc = QJsonDocument(data.toObject());
    }
    else if (data.isArray()) {
        doc = QJsonDocument(data.toArray());
    }

    QByteArray jsonData = doc.toJson(QJsonDocument::Compact);

    QNetworkReply* reply = m_networkManager->post(request, jsonData);

    QEventLoop loop;
    QTimer timer;
    timer.setSingleShot(true);

    QObject::connect(reply, &QNetworkReply::finished, &loop, &QEventLoop::quit);
    QObject::connect(&timer, &QTimer::timeout, &loop, &QEventLoop::quit);

    timer.start(10000);
    loop.exec();

    if (!timer.isActive()) {
        setLastError("Request timeout");
        reply->deleteLater();
        return false;
    }
    timer.stop();

    if (reply->error() == QNetworkReply::AuthenticationRequiredError) {
        reply->deleteLater();

        if (retryCount < 1) {
            std::cout << u8"\n⚠️ Authentication error, attempting token refresh...";
            if (refreshAccessToken()) {
                std::cout << u8"\n🔄 Retrying request after token refresh...";
                return performPostRequest(path, data, retryCount + 1);
            }
        }

        setLastError("Authentication expired. Please login again.");
        m_isAuthenticated = false;
        emit authenticationRequired();
        return false;
    }

    bool success = (reply->error() == QNetworkReply::NoError);
    if (!success) {
        setLastError(reply->errorString());
    }
    else {
        setLastError(QString());
    }

    reply->deleteLater();
    return success;
}

// ==================== DELETE REQUEST ====================
bool FirebaseDatabaseManager::performDeleteRequest(const QString& path, int retryCount)
{
    if (!m_isAuthenticated) {
        setLastError("Not authenticated. Please log in first.");
        emit authenticationRequired();
        return false;
    }

    QUrl url(buildUrl(path));
    QNetworkRequest request(url);

    QNetworkReply* reply = m_networkManager->deleteResource(request);

    QEventLoop loop;
    QTimer timer;
    timer.setSingleShot(true);

    QObject::connect(reply, &QNetworkReply::finished, &loop, &QEventLoop::quit);
    QObject::connect(&timer, &QTimer::timeout, &loop, &QEventLoop::quit);

    timer.start(10000);
    loop.exec();

    if (!timer.isActive()) {
        setLastError("Request timeout");
        reply->deleteLater();
        return false;
    }
    timer.stop();

    if (reply->error() == QNetworkReply::AuthenticationRequiredError) {
        reply->deleteLater();

        if (retryCount < 1) {
            std::cout << u8"\n⚠️ Authentication error, attempting token refresh...";
            if (refreshAccessToken()) {
                std::cout << u8"\n🔄 Retrying request after token refresh...";
                return performPostRequest(path, retryCount + 1);
            }
        }

        setLastError("Authentication expired. Please login again.");
        m_isAuthenticated = false;
        emit authenticationRequired();
        return false;
    }

    bool success = (reply->error() == QNetworkReply::NoError);
    if (!success) {
        setLastError(reply->errorString());
    }
    else {
        setLastError(QString());
    }

    reply->deleteLater();
    return success;
}

// ==================== PATCH REQUEST ====================
bool FirebaseDatabaseManager::performPatchRequest(const QString& path, const QJsonValue& data, int retryCount)
{
    if (!m_isAuthenticated) {
        setLastError("Not authenticated. Please log in first.");
        emit authenticationRequired();
        return false;
	}

    QUrl url(buildUrl(path));
    QNetworkRequest request(url);
    request.setHeader(QNetworkRequest::ContentTypeHeader, "application/json");

    QJsonDocument doc;
    if (data.isObject()) {
        doc = QJsonDocument(data.toObject());
    }

    QByteArray jsonData = doc.toJson(QJsonDocument::Compact);

    // Qt non ha un metodo patch() diretto, quindi usiamo sendCustomRequest
    QNetworkReply* reply = m_networkManager->sendCustomRequest(request, "PATCH", jsonData);

    QEventLoop loop;
    QTimer timer;
    timer.setSingleShot(true);

    QObject::connect(reply, &QNetworkReply::finished, &loop, &QEventLoop::quit);
    QObject::connect(&timer, &QTimer::timeout, &loop, &QEventLoop::quit);

    timer.start(10000);
    loop.exec();

    if (!timer.isActive()) {
        setLastError("Request timeout");
        reply->deleteLater();
        return false;
    }
    timer.stop();

    if (reply->error() == QNetworkReply::AuthenticationRequiredError) {
        reply->deleteLater();

        if (retryCount < 1) {
            std::cout << u8"\n⚠️ Authentication error, attempting token refresh...";
            if (refreshAccessToken()) {
                std::cout << u8"\n🔄 Retrying request after token refresh...";
                return performPostRequest(path, data, retryCount + 1);
            }
        }

        setLastError("Authentication expired. Please login again.");
        m_isAuthenticated = false;
        emit authenticationRequired();
        return false;
    }

    bool success = (reply->error() == QNetworkReply::NoError);
    if (!success) {
        setLastError(reply->errorString());
    }
    else {
        setLastError(QString());
    }

    reply->deleteLater();
    return success;
}

// ==================== CONVERSIONE OGGETTI ====================
QJsonObject FirebaseDatabaseManager::objectToJson(const HomeObject& object) const
{
    QJsonObject json;
    json["name"] = object.name();
    json["color"] = object.color();
    json["material"] = object.material();
    json["type"] = object.type();
    json["notes"] = object.notes();
    json["locationId"] = object.locationId();
    json["sublocationId"] = object.sublocationId();

    // Picture come base64 se presente
    if (!object.picture().isEmpty()) {
        json["picture"] = QString(object.picture().toBase64());
    }

    return json;
}

HomeObject FirebaseDatabaseManager::jsonToObject(const QString& key, const QJsonObject& json) const
{
    HomeObject obj;
    obj.setName(json["name"].toString());
    obj.setColor(json["color"].toString());
    obj.setMaterial(json["material"].toString());
    obj.setType(json["type"].toString());
    obj.setNotes(json["notes"].toString());
    obj.setLocationId(json["locationId"].toInt());
    obj.setSublocationId(json["sublocationId"].toInt());

    // Picture da base64 se presente
    if (json.contains("picture")) {
        QByteArray pictureData = QByteArray::fromBase64(json["picture"].toString().toUtf8());
        obj.setPicture(pictureData);
    }

    return obj;
}

// ==================== OBJECT OPERATIONS ====================

bool FirebaseDatabaseManager::createObject(const HomeObject& object)
{
    if (!isAuthenticated()) {
		setLastError("Not authenticated");
        emit authenticationRequired();
		return false;   
    }

    if (!object.isValid()) {
        setLastError("Invalid object - name is required");
        return false;
    }

    QJsonObject jsonObj = objectToJson(object);
    QString path = QString("/objects/%1.json").arg(object.name());

    bool success = performPutRequest(path, jsonObj);

    if (success) {
        std::cout << u8"✅ Object created: ";
        qDebug() << object.name();
    }

    return success;
}

QList<HomeObject> FirebaseDatabaseManager::getAllObjects()
{
    QList<HomeObject> objects;

    if (!isAuthenticated()) {
        setLastError("Not authenticated");
        emit authenticationRequired();
        return objects;
    }

    QJsonDocument doc = performGetRequest("/objects.json");

    if (doc.isNull() || !doc.isObject()) {
        return objects; // Empty list se non ci sono oggetti
    }

    QJsonObject allObjects = doc.object();

    for (auto it = allObjects.begin(); it != allObjects.end(); ++it) {
        QString key = it.key();
        QJsonObject objJson = it.value().toObject();

        HomeObject obj = jsonToObject(key, objJson);
        objects.append(obj);
    }

    std::cout << u8"📦 Retrieved " << objects.size() << " objects from Firebase\n";
    return objects;
}

QList<HomeObject> FirebaseDatabaseManager::getObjects(int locationId, int sublocationId)
{
    QList<HomeObject> allObjects = getAllObjects();
    QList<HomeObject> filteredObjects;

    for (const HomeObject& obj : allObjects) {
        if (obj.locationId() == locationId && obj.sublocationId() == sublocationId) {
            filteredObjects.append(obj);
        }
    }

    return filteredObjects;
}

bool FirebaseDatabaseManager::updateObject(const QString& oldName, const HomeObject& newObject)
{
    if (!isAuthenticated()) {
        setLastError("Not authenticated");
        emit authenticationRequired();
        return false;
    }

    // Se il nome è cambiato, devo eliminare il vecchio e creare il nuovo
    if (oldName != newObject.name()) {
        if (!deleteObject(oldName)) {
            return false;
        }
    }

    return createObject(newObject);
}

bool FirebaseDatabaseManager::deleteObject(const QString& objectName)
{
    if (!isAuthenticated()) {
        setLastError("Not authenticated");
        emit authenticationRequired();
        return false;
    }

    QString path = QString("/objects/%1.json").arg(objectName);
    bool success = performDeleteRequest(path);

    if (success) {
        std::cout << u8"🗑️ Object deleted: ";
        qDebug() << objectName;
    }

    return success;
}

QList<HomeObject> FirebaseDatabaseManager::searchObjects(const QVariantMap& filters)
{
    QList<HomeObject> allObjects = getAllObjects();
    QList<HomeObject> results;

    for (const HomeObject& obj : allObjects) {
        bool matches = true;

        // Filtra per nome (partial match)
        if (filters.contains("name")) {
            QString searchName = filters["name"].toString();
            if (!searchName.isEmpty() && !obj.name().contains(searchName, Qt::CaseInsensitive)) {
                matches = false;
            }
        }

        // Filtra per colore
        if (filters.contains("colors")) {
            QStringList colors = filters["colors"].toStringList();
            if (!colors.isEmpty() && !colors.contains(obj.color())) {
                matches = false;
            }
        }

        // Filtra per materiale
        if (filters.contains("materials")) {
            QStringList materials = filters["materials"].toStringList();
            if (!materials.isEmpty() && !materials.contains(obj.material())) {
                matches = false;
            }
        }

        // Filtra per tipo
        if (filters.contains("types")) {
            QStringList types = filters["types"].toStringList();
            if (!types.isEmpty() && !types.contains(obj.type())) {
                matches = false;
            }
        }

        if (matches) {
            results.append(obj);
        }
    }

    return results;
}

// ==================== ATTRIBUTES OPERATIONS ====================

QStringList FirebaseDatabaseManager::getColors()
{
    QStringList colors;

    if (!isAuthenticated()) {
        setLastError("Not authenticated");
        emit authenticationRequired();
        return colors;
    }

    QJsonDocument doc = performGetRequest("/colors.json");

    if (doc.isArray()) {
        QJsonArray arr = doc.array();
        for (const QJsonValue& val : arr) {
            colors.append(val.toString());
        }
    }

    std::cout << u8"🎨 Retrieved " << colors.size() << " colors\n";
    return colors;
}

QStringList FirebaseDatabaseManager::getMaterials()
{
    QStringList materials;

    if (!isAuthenticated()) {
        setLastError("Not authenticated");
        emit authenticationRequired();
        return materials;
    }

    QJsonDocument doc = performGetRequest("/materials.json");

    if (doc.isArray()) {
        QJsonArray arr = doc.array();
        for (const QJsonValue& val : arr) {
            materials.append(val.toString());
        }
    }

    std::cout << u8"🔨 Retrieved " << materials.size() << " materials\n";
    return materials;
}

QStringList FirebaseDatabaseManager::getTypes()
{
    QStringList types;

    if (!isAuthenticated()) {
        setLastError("Not authenticated");
        emit authenticationRequired();
        return types;
    }

    QJsonDocument doc = performGetRequest("/types.json");

    if (doc.isArray()) {
        QJsonArray arr = doc.array();
        for (const QJsonValue& val : arr) {
            types.append(val.toString());
        }
    }

    std::cout << u8"📋 Retrieved " << types.size() << " types\n";
    return types;
}

bool FirebaseDatabaseManager::addColor(const QString& color)
{
    if (!isAuthenticated()) {
        setLastError("Not authenticated");
        emit authenticationRequired();
        return false;
    }

    QStringList colors = getColors();

    if (colors.contains(color)) {
        setLastError("Color already exists");
        return false;
    }

    colors.append(color);

    QJsonArray arr;
    for (const QString& c : colors) {
        arr.append(c);
    }

    return performPutRequest("/colors.json", arr);
}

bool FirebaseDatabaseManager::addMaterial(const QString& material)
{
    if (!isAuthenticated()) {
        setLastError("Not authenticated");
        emit authenticationRequired();
        return false;
    }

    QStringList materials = getMaterials();

    if (materials.contains(material)) {
        setLastError("Material already exists");
        return false;
    }

    materials.append(material);

    QJsonArray arr;
    for (const QString& m : materials) {
        arr.append(m);
    }

    return performPutRequest("/materials.json", arr);
}

bool FirebaseDatabaseManager::addType(const QString& type)
{
    if (!isAuthenticated()) {
        setLastError("Not authenticated");
        emit authenticationRequired();
        return false;
    }

    QStringList types = getTypes();

    if (types.contains(type)) {
        setLastError("Type already exists");
        return false;
    }

    types.append(type);

    QJsonArray arr;
    for (const QString& t : types) {
        arr.append(t);
    }

    return performPutRequest("/types.json", arr);
}

bool FirebaseDatabaseManager::removeColor(const QString& color)
{
    if (!isAuthenticated()) {
        setLastError("Not authenticated");
        emit authenticationRequired();
        return false;
    }

    QStringList colors = getColors();
    colors.removeAll(color);

    QJsonArray arr;
    for (const QString& c : colors) {
        arr.append(c);
    }

    return performPutRequest("/colors.json", arr);
}

bool FirebaseDatabaseManager::removeMaterial(const QString& material)
{
    if (!isAuthenticated()) {
        setLastError("Not authenticated");
        emit authenticationRequired();
        return false;
    }

    QStringList materials = getMaterials();
    materials.removeAll(material);

    QJsonArray arr;
    for (const QString& m : materials) {
        arr.append(m);
    }

    return performPutRequest("/materials.json", arr);
}

bool FirebaseDatabaseManager::removeType(const QString& type)
{
    if (!isAuthenticated()) {
        setLastError("Not authenticated");
        emit authenticationRequired();
        return false;
    }

    QStringList types = getTypes();
    types.removeAll(type);

    QJsonArray arr;
    for (const QString& t : types) {
        arr.append(t);
    }

    return performPutRequest("/types.json", arr);
}