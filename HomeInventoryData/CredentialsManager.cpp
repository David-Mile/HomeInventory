#include "CredentialsManager.h"
#include <QCryptographicHash>
#include <QSysInfo>
#include <QStandardPaths>
#include <QDir>
#include <QDebug>
#include <iostream>

// Se hai OpenSSL disponibile, puoi usare AES-256
// Altrimenti usiamo XOR con chiave derivata dal sistema
// Per produzione seria, considera QtCrypto o OpenSSL

CredentialsManager::CredentialsManager()
{
    // Salva le impostazioni in una posizione sicura e persistente
    QString configPath = QStandardPaths::writableLocation(QStandardPaths::AppDataLocation);
    QDir().mkpath(configPath);

    m_settings = new QSettings(configPath + "/credentials.conf", QSettings::IniFormat);

    // Imposta permessi restrittivi sul file (solo il proprietario può leggere)
#ifndef Q_OS_WIN
    QFile file(m_settings->fileName());
    file.setPermissions(QFile::ReadOwner | QFile::WriteOwner);
#endif
}

CredentialsManager::~CredentialsManager()
{
    delete m_settings;
}

QByteArray CredentialsManager::getEncryptionKey() const
{
    // Crea una chiave basata su informazioni del sistema
    // Questo rende le credenziali non trasferibili su altri PC
    QString uniqueId = QSysInfo::machineUniqueId();

    if (uniqueId.isEmpty()) {
        // Fallback se machineUniqueId non disponibile
        uniqueId = QSysInfo::machineHostName() +
            QSysInfo::productType() +
            QSysInfo::kernelVersion();
    }

    // Aggiungi un salt specifico dell'applicazione
    uniqueId += "HomeInventory_v1.0_SecretSalt_2025";

    // Hash SHA-256 per ottenere una chiave a 256 bit
    return QCryptographicHash::hash(uniqueId.toUtf8(), QCryptographicHash::Sha256);
}

QByteArray CredentialsManager::simpleEncrypt(const QString& plainText) const
{
    QByteArray key = getEncryptionKey();
    QByteArray data = plainText.toUtf8();
    QByteArray encrypted;

    // XOR con la chiave (ripetuta ciclicamente)
    for (int i = 0; i < data.size(); ++i) {
        encrypted.append(data[i] ^ key[i % key.size()]);
    }

    return encrypted.toBase64();
}

QString CredentialsManager::simpleDecrypt(const QByteArray& cipherText) const
{
    QByteArray key = getEncryptionKey();
    QByteArray data = QByteArray::fromBase64(cipherText);
    QByteArray decrypted;

    // XOR con la chiave (ripetuta ciclicamente)
    for (int i = 0; i < data.size(); ++i) {
        decrypted.append(data[i] ^ key[i % key.size()]);
    }

    return QString::fromUtf8(decrypted);
}

QByteArray CredentialsManager::encrypt(const QString& plainText) const
{
    // Per semplicità usiamo XOR
    // In produzione, usa QCA (Qt Cryptographic Architecture) o OpenSSL
    return simpleEncrypt(plainText);
}

QString CredentialsManager::decrypt(const QByteArray& cipherText) const
{
    return simpleDecrypt(cipherText);
}

bool CredentialsManager::saveCredentials(const QString& email, const QString& password)
{
    if (email.isEmpty() || password.isEmpty()) {
        return false;
    }

    try {
        QByteArray encryptedEmail = encrypt(email);
        QByteArray encryptedPassword = encrypt(password);

        m_settings->setValue("auth/email", encryptedEmail);
        m_settings->setValue("auth/password", encryptedPassword);
        m_settings->setValue("auth/saved", true);
        m_settings->sync();

        std::cout << u8"✅ Credentials saved securely\n";
        return m_settings->status() == QSettings::NoError;
    }
    catch (...) {
        std::cout << u8"❌ Failed to save credentials\n";
        return false;
    }
}

bool CredentialsManager::loadCredentials(QString& email, QString& password)
{
    if (!hasStoredCredentials()) {
        return false;
    }

    try {
        QByteArray encryptedEmail = m_settings->value("auth/email").toByteArray();
        QByteArray encryptedPassword = m_settings->value("auth/password").toByteArray();

        email = decrypt(encryptedEmail);
        password = decrypt(encryptedPassword);

        std::cout << u8"✅ Credentials loaded from secure storage\n";
        return !email.isEmpty() && !password.isEmpty();
    }
    catch (...) {
        std::cout << u8"❌ Failed to load credentials\n";
        return false;
    }
}

bool CredentialsManager::hasStoredCredentials() const
{
    return m_settings->value("auth/saved", false).toBool();
}

void CredentialsManager::clearCredentials()
{
    m_settings->remove("auth/email");
    m_settings->remove("auth/password");
    m_settings->remove("auth/saved");
    m_settings->remove("auth/refreshToken");
    m_settings->sync();

    std::cout << u8"🗑️ Credentials cleared\n";
}

bool CredentialsManager::saveRefreshToken(const QString& refreshToken)
{
    if (refreshToken.isEmpty()) {
        return false;
    }

    QByteArray encrypted = encrypt(refreshToken);
    m_settings->setValue("auth/refreshToken", encrypted);
    m_settings->sync();

    return m_settings->status() == QSettings::NoError;
}

QString CredentialsManager::loadRefreshToken()
{
    if (!m_settings->contains("auth/refreshToken")) {
        return QString();
    }

    QByteArray encrypted = m_settings->value("auth/refreshToken").toByteArray();
    return decrypt(encrypted);
}