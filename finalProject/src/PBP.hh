#ifndef PBP_H
#define PBP_H

#include <QtCrypto>
#include <QByteArray>
#include <QDebug>

QByteArray encryptMap(QVariantMap, QCA::PublicKey, QCA::PrivateKey);
QVariantMap decryptMap(QVariantMap, QCA::PublicKey, QCA::PrivateKey);

#endif
