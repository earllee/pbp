#ifndef PBP_H
#define PBP_H

#include <QtCrypto>
#include <QByteArray>
#include <QDebug>

// In: datagram of msg to send, pubKey of recipient, privKey of self
// Returns QByteArray, datagram that represents QVariantMap containing
// - original datagram encrypted by random sym key with AES-128 cipher
// - sym key encrypted by recipient's public key, RSA-1028
// - Digest of original datagram, hashed by SHA-1, signed w/ RSA-1028 own privKey
// - Initialization vector for the AES cipher, unsecured
QByteArray encryptDatagram(QByteArray datagram, QCA::PublicKey pubKey, 
    QCA::PrivateKey privKey); 

QByteArray decryptDatagram(QByteArray secureMsg, QCA::PublicKey pubKey, 
    QCA::PrivateKey privKey); 

QByteArray encryptMap(QVariantMap, QCA::PublicKey, QCA::PrivateKey);

QVariantMap decryptData(QByteArray, QCA::PublicKey, QCA::PrivateKey);

#endif
