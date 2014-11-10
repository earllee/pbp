#include "encrypt.h"

// In: datagram of msg to send, pubKey of recipient, privKey of self
// Returns QByteArray, datagram that represents QVariantMap containing
// - original datagram encrypted by random sym key with AES-128 cipher
// - sym key encrypted by recipient's public key, RSA-1028
// - Digest of original datagram, hashed by SHA-1, signed w/ RSA-1028 own privKey
// - Initialization vector for the AES cipher, unsecured
QByteArray encryptDatagram(QByteArray datagram, QCA::PublicKey pubKey, 
    QCA::PrivateKey privKey) 
{
    QCA::SymmetricKey symKey(16); // 128 bit random symmetric key
    QCA::InitializationVector iV(16); // need initVector to randomize input
    QCA::Cipher cipher(QString("aes128"), QCA::Cipher::CBC,
        QCA::Cipher::DefaultPadding, QCA::Encode, symKey, iV);
    QCA::MemoryRegion dataMemCpy(datagram), dataMemOut(datagram);
    dataMemOut = cipher.update(dataMemOut);
    dataMemOut = cipher.final();    // Encrypt datagram w/ symKey, padded

    // Hash the contents of datagram
    QByteArray dataDigest = QCA::Hash("sha1").hash(dataMemCpy).toByteArray();
    
    // Sign hash
    QCA::SecureArray secureDigest;
    privKey.decrypt(QCA::MemoryRegion(dataDigest), &secureDigest, QCA::EME_PKCS1_OAEP);
    
    // Encrypt symmetric key
    QCA::MemoryRegion secureSymKey = pubKey.encrypt(symKey, QCA::EME_PKCS1_OAEP);

    // Populate contents of new QVariantMap; may need to extract some fields
    QVariantMap secureMsg;
    secureMsg["secureSymKey"] = secureSymKey.toByteArray();
    secureMsg["secureDatagram"] = dataMemOut.toByteArray();
    secureMsg["secureDigest"] = secureDigest.toByteArray();
    secureMsg["iV"] = iV.toByteArray();
    
    // Reconvert to byte array/datagram
    QByteArray secureDatagram;
    QDataStream outStream(&secureDatagram, QIODevice::WriteOnly);
    outStream << secureMsg;

    return secureDatagram;
}
