#pragma once
#ifndef FIREBASEDATABASEMANAGER_H
#define FIREBASEDATABASEMANAGER_H

#include "idatabasemanager.h"
#include "homeinventorydata_global.h"
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

private:
    QString m_firebaseUrl;
    bool m_isConnected;
    QNetworkAccessManager* m_networkManager;
    QString m_lastError;

    // Helper methods
    QString buildUrl(const QString& path) const;
    QJsonDocument performGetRequest(const QString& path);
    bool performPutRequest(const QString& path, const QJsonValue& data);
    bool performPostRequest(const QString& path, const QJsonValue& data);
    bool performDeleteRequest(const QString& path);
    bool performPatchRequest(const QString& path, const QJsonValue& data);

    QJsonObject objectToJson(const HomeObject& object) const;
    HomeObject jsonToObject(const QString& key, const QJsonObject& json) const;

    void setLastError(const QString& error);
};

#endif // FIREBASEDATABASEMANAGER_H