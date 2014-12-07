#ifndef PBP_H
#define PBP_H

#include <QtCrypto>
#include <QByteArray>
#include <QDebug>

void encryptMap(QVariantMap&, QCA::PublicKey, QCA::PrivateKey);
void decryptMap(QVariantMap&, QCA::PublicKey, QCA::PrivateKey);

#endif
