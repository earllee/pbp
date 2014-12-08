#ifndef PBP_H
#define PBP_H

#include <QtCrypto>
#include <QByteArray>
#include <QDebug>

bool encryptMap(QVariantMap&, QCA::PublicKey, QCA::PrivateKey);
bool decryptMap(QVariantMap&, QCA::PublicKey, QCA::PrivateKey);

#endif
