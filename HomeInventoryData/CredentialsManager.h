#pragma once
#ifndef CREDENTIALSMANAGER_H
#define CREDENTIALSMANAGER_H

#include <QString>
#include <QByteArray>
#include <QSettings>

/**
 * @brief Classe per salvare e recuperare credenziali in modo sicuro
 * Le credenziali vengono criptate con AES-256 usando una chiave derivata dal sistema
 */
class CredentialsManager
{
public:
    CredentialsManager();
    ~CredentialsManager();

    // Salva le credenziali in modo criptato
    bool saveCredentials(const QString& email, const QString& password);

    // Recupera le credenziali salvate
    bool loadCredentials(QString& email, QString& password);

    // Verifica se esistono credenziali salvate
    bool hasStoredCredentials() const;

    // Elimina le credenziali salvate (logout completo)
    void clearCredentials();

    // Salva il refresh token per sessioni lunghe
    bool saveRefreshToken(const QString& refreshToken);
    QString loadRefreshToken();

private:
    QSettings* m_settings;

    // Chiave di criptazione derivata dal sistema
    QByteArray getEncryptionKey() const;

    // Criptazione/Decriptazione AES-256
    QByteArray encrypt(const QString& plainText) const;
    QString decrypt(const QByteArray& cipherText) const;

    // XOR semplice come fallback (se OpenSSL non disponibile)
    QByteArray simpleEncrypt(const QString& plainText) const;
    QString simpleDecrypt(const QByteArray& cipherText) const;
};

#endif // CREDENTIALSMANAGER_H
