#include "firebasedatabasemanager.h"
#include "HomeObject.h"
#include <QNetworkRequest>
#include <QJsonDocument>
#include <QJsonObject>
#include <QJsonArray>
#include <QUrlQuery>
#include <QDebug>
#include <iostream>

FirebaseDatabaseManager::FirebaseDatabaseManager(QObject* parent)
    : QObject(parent)
    , m_isConnected(false)
    , m_networkManager(new QNetworkAccessManager(this))
{
}

FirebaseDatabaseManager::~FirebaseDatabaseManager()
{
    disconnect();
}

bool FirebaseDatabaseManager::connect(const QString& firebaseUrl)
{
    m_firebaseUrl = "https://homeinventory-b858b-default-rtdb.europe-west1.firebasedatabase.app/";

    // Rimuovi trailing slash se presente
    if (m_firebaseUrl.endsWith('/')) {
        m_firebaseUrl.chop(1);
    }

    // Test della connessione con una semplice GET request
    QJsonDocument testDoc = performGetRequest("/.json");

    if (!testDoc.isNull()) {
        m_isConnected = true;
        setLastError(QString());
        std::cout << u8"✅ Connected to Firebase:";
        qDebug() << m_firebaseUrl;
        return true;
    }

    m_isConnected = false;
    setLastError("Failed to connect to Firebase database");
    std::cout << u8"❌ Failed to connect to Firebase:";
    qDebug() << m_firebaseUrl;
    return false;
}

void FirebaseDatabaseManager::disconnect()
{
    m_isConnected = false;
    m_firebaseUrl.clear();
}

bool FirebaseDatabaseManager::isConnected() const
{
    return m_isConnected;
}

QString FirebaseDatabaseManager::lastError() const
{
    return m_lastError;
}

void FirebaseDatabaseManager::setLastError(const QString& error)
{
    m_lastError = error;
    if (!error.isEmpty()) {
        qDebug() << "❌ Firebase Error:" << error;
    }
}

QString FirebaseDatabaseManager::buildUrl(const QString& path) const
{
    return m_firebaseUrl + path;
}

// ==================== GET REQUEST ====================
QJsonDocument FirebaseDatabaseManager::performGetRequest(const QString& path)
{
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
bool FirebaseDatabaseManager::performPutRequest(const QString& path, const QJsonValue& data)
{
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
bool FirebaseDatabaseManager::performPostRequest(const QString& path, const QJsonValue& data)
{
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
bool FirebaseDatabaseManager::performDeleteRequest(const QString& path)
{
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
bool FirebaseDatabaseManager::performPatchRequest(const QString& path, const QJsonValue& data)
{
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
    if (!isConnected()) {
        setLastError("Not connected to database");
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
        qDebug() << "✅ Object created:" << object.name();
    }

    return success;
}

QList<HomeObject> FirebaseDatabaseManager::getAllObjects()
{
    QList<HomeObject> objects;

    if (!isConnected()) {
        setLastError("Not connected to database");
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

    qDebug() << "📦 Retrieved" << objects.size() << "objects from Firebase";
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
    if (!isConnected()) {
        setLastError("Not connected to database");
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
    if (!isConnected()) {
        setLastError("Not connected to database");
        return false;
    }

    QString path = QString("/objects/%1.json").arg(objectName);
    bool success = performDeleteRequest(path);

    if (success) {
        qDebug() << "🗑️ Object deleted:" << objectName;
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

    if (!isConnected()) {
        setLastError("Not connected to database");
        return colors;
    }

    QJsonDocument doc = performGetRequest("/colors.json");

    if (doc.isArray()) {
        QJsonArray arr = doc.array();
        for (const QJsonValue& val : arr) {
            colors.append(val.toString());
        }
    }

    qDebug() << "🎨 Retrieved" << colors.size() << "colors";
    return colors;
}

QStringList FirebaseDatabaseManager::getMaterials()
{
    QStringList materials;

    if (!isConnected()) {
        setLastError("Not connected to database");
        return materials;
    }

    QJsonDocument doc = performGetRequest("/materials.json");

    if (doc.isArray()) {
        QJsonArray arr = doc.array();
        for (const QJsonValue& val : arr) {
            materials.append(val.toString());
        }
    }

    qDebug() << "🔨 Retrieved" << materials.size() << "materials";
    return materials;
}

QStringList FirebaseDatabaseManager::getTypes()
{
    QStringList types;

    if (!isConnected()) {
        setLastError("Not connected to database");
        return types;
    }

    QJsonDocument doc = performGetRequest("/types.json");

    if (doc.isArray()) {
        QJsonArray arr = doc.array();
        for (const QJsonValue& val : arr) {
            types.append(val.toString());
        }
    }

    qDebug() << "📋 Retrieved" << types.size() << "types";
    return types;
}

bool FirebaseDatabaseManager::addColor(const QString& color)
{
    if (!isConnected()) {
        setLastError("Not connected to database");
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
    if (!isConnected()) {
        setLastError("Not connected to database");
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
    if (!isConnected()) {
        setLastError("Not connected to database");
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
    if (!isConnected()) {
        setLastError("Not connected to database");
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
    if (!isConnected()) {
        setLastError("Not connected to database");
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
    if (!isConnected()) {
        setLastError("Not connected to database");
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