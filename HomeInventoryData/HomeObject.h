#pragma once
#ifndef HOMEOBJECT_H
#define HOMEOBJECT_H

#include "homeinventorydata_global.h"
#include <QString>
#include <QByteArray>

class HOMEINVENTORYDATA_EXPORT HomeObject
{
public:
    HomeObject();
    HomeObject(const QString& name, int locationId, int sublocationId);

    // Getters
    QString name() const { return m_name; }
    QString color() const { return m_color; }
    QString material() const { return m_material; }
    QString type() const { return m_type; }
    QString notes() const { return m_notes; }
    QByteArray picture() const { return m_picture; }
    int locationId() const { return m_locationId; }
    int sublocationId() const { return m_sublocationId; }

    // Setters
    void setName(const QString& name) { m_name = name; }
    void setColor(const QString& color) { m_color = color; }
    void setMaterial(const QString& material) { m_material = material; }
    void setType(const QString& type) { m_type = type; }
    void setNotes(const QString& notes) { m_notes = notes; }
    void setPicture(const QByteArray& picture) { m_picture = picture; }
    void setLocationId(int id) { m_locationId = id; }
    void setSublocationId(int id) { m_sublocationId = id; }

    // Validation
    bool isValid() const;

    // Comparison
    bool operator==(const HomeObject& other) const;
    bool operator!=(const HomeObject& other) const;

private:
    QString m_name;
    QString m_color;
    QString m_material;
    QString m_type;
    QString m_notes;
    QByteArray m_picture;
    int m_locationId;
    int m_sublocationId;
};

#endif // HOMEOBJECT_H

