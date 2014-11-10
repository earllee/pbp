#ifndef ENCRYPT_H
#define ENCRYPT_H

#include <QtCrypto>
#include <QByteArray>

// In: datagram of msg to send, pubKey of recipient, privKey of self
// Returns QByteArray, datagram that represents QVariantMap containing
// - original datagram encrypted by random sym key with AES-128 cipher
// - sym key encrypted by recipient's public key, RSA-1028
// - Digest of original datagram, hashed by SHA-1, signed w/ RSA-1028 own privKey
// - Initialization vector for the AES cipher, unsecured
QByteArray encryptDatagram(QByteArray datagram, QCA::PublicKey pubKey, 
    QCA::PrivateKey privKey); 

#endif

