#include "HomeObject.h"

HomeObject::HomeObject()
    : m_locationId(0)
    , m_sublocationId(0)
{
}

HomeObject::HomeObject(const QString& name, int locationId, int sublocationId)
    : m_name(name)
    , m_locationId(locationId)
    , m_sublocationId(sublocationId)
{
}

bool HomeObject::isValid() const
{
    // Un oggetto è valido se ha almeno un nome non vuoto
    return !m_name.isEmpty();
}

bool HomeObject::operator==(const HomeObject& other) const
{
    return m_name == other.m_name &&
        m_locationId == other.m_locationId &&
        m_sublocationId == other.m_sublocationId;
}

bool HomeObject::operator!=(const HomeObject& other) const
{
    return !(*this == other);
}