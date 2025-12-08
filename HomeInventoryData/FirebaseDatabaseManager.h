#pragma once
#ifndef FIREBASEDATABASEMANAGER_H
#define FIREBASEDATABASEMANAGER_H

#include "idatabasemanager.h"
#include "homeinventorydata_global.h"
#include "CredentialsManager.h"
#include <QObject>
#include <QNetworkAccessManager>
#include <QNetworkReply>
#include <QJsonDocument>
#include <QJsonObject>
#include <QJsonArray>
#include <QEventLoop>
#include <QTimer>

class HOMEINVENTORYDATA_EXPORT FirebaseDatabaseManager : public QObject, public IDatabaseManager
{
    Q_OBJECT

public:
    explicit FirebaseDatabaseManager(QObject* parent = nullptr);
    ~FirebaseDatabaseManager() override;

    // IDatabaseManager interface
    bool connect(const QString& firebaseUrl) override;
    void disconnect() override;
    bool isConnected() const override;

	// Authentication methods
    bool setApiKey(const QString& apiKey);
    bool authenticateWithEmail(const QString& email, const QString& password, bool rememberMe = true);
    bool tryAutoLogin(); // Try auto login with saved credentials
    bool isAuthenticated() const;
    QString currentUserEmail() const;
    void logout(bool clearSavedCredentials = false);

    bool createObject(const HomeObject& object) override;
    QList<HomeObject> getObjects(int locationId, int sublocationId) override;
    QList<HomeObject> getAllObjects() override;
    bool updateObject(const QString& oldName, const HomeObject& newObject) override;
    bool deleteObject(const QString& objectName) override;

    QList<HomeObject> searchObjects(const QVariantMap& filters) override;

    QStringList getColors() override;
    QStringList getMaterials() override;
    QStringList getTypes() override;

    bool addColor(const QString& color) override;
    bool addMaterial(const QString& material) override;
    bool addType(const QString& type) override;

    bool removeColor(const QString& color) override;
    bool removeMaterial(const QString& material) override;
    bool removeType(const QString& type) override;

    QString lastError() const override;

signals:
    void authenticationCompleted(bool success, const QString& email);
    void authenticationRequired();

private:
    QString m_firebaseUrl;
	QString m_apiKey;
    bool m_isConnected;
	bool m_isAuthenticated;
    QString m_idToken;
    QString m_refreshToken;
    QString m_userEmail;
    QString m_userId;
    QNetworkAccessManager* m_networkManager;
    QString m_lastError;
	CredentialsManager* m_credentialsManager;
	QDateTime m_tokenExpiry;
	bool m_isRefreshingToken; // prevent infinite token refresh loops

    // Helper methods
    QString buildUrl(const QString& path) const;
    QJsonDocument performGetRequest(const QString& path, int retryCount = 0);
    bool performPutRequest(const QString& path, const QJsonValue& data, int retryCount = 0);
    bool performPostRequest(const QString& path, const QJsonValue& data, int retryCount = 0);
    bool performDeleteRequest(const QString& path, int retryCount = 0);
    bool performPatchRequest(const QString& path, const QJsonValue& data, int retryCount = 0);

    // Auth helper methods
    bool signInWithEmailPassword(const QString& email, const QString& password);
    bool refreshAccessToken();
    bool verifyIdToken();

    QJsonObject objectToJson(const HomeObject& object) const;
    HomeObject jsonToObject(const QString& key, const QJsonObject& json) const;

    void setLastError(const QString& error);
};

#endif // FIREBASEDATABASEMANAGER_H