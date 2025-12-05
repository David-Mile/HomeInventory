#pragma once
#ifndef IDATABASEMANAGER_H
#define IDATABASEMANAGER_H

#include "homeinventorydata_global.h"
#include <QString>
#include <QList>
#include <QVariantMap>

// Forward declaration
class HomeObject;

/**
 * @brief Interface astratta per la gestione del database
 * Permette di cambiare implementazione senza modificare il codice client
 */
class HOMEINVENTORYDATA_EXPORT IDatabaseManager
{
public:
    virtual ~IDatabaseManager() = default;

    // Connection Management
    virtual bool connect(const QString& connectionString) = 0;
    virtual void disconnect() = 0;
    virtual bool isConnected() const = 0;

    // Object Operations (CRUD)
    virtual bool createObject(const HomeObject& object) = 0;
    virtual QList<HomeObject> getObjects(int locationId, int sublocationId) = 0;
    virtual QList<HomeObject> getAllObjects() = 0;
    virtual bool updateObject(const QString& oldName, const HomeObject& newObject) = 0;
    virtual bool deleteObject(const QString& objectName) = 0;

    // Search Operations
    virtual QList<HomeObject> searchObjects(const QVariantMap& filters) = 0;

    // Attribute Operations
    virtual QStringList getColors() = 0;
    virtual QStringList getMaterials() = 0;
    virtual QStringList getTypes() = 0;

    virtual bool addColor(const QString& color) = 0;
    virtual bool addMaterial(const QString& material) = 0;
    virtual bool addType(const QString& type) = 0;

    virtual bool removeColor(const QString& color) = 0;
    virtual bool removeMaterial(const QString& material) = 0;
    virtual bool removeType(const QString& type) = 0;

    // Error handling
    virtual QString lastError() const = 0;
};

#endif // IDATABASEMANAGER_H