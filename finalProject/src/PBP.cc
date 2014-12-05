#include "PBP.hh"

#define SYMMETRIC_KEY_SIZE 16

// Takes in a QVariantMap DATA and returns a QByteArray of the encrypted map. 
// Original data is encrypted using symmetric key with AES-128 cipher. 
// Symmetric key is encrypted by recipient's public key using RSA-1028. Digest 
// of original data is hashed using SHA-1 and signed with RSA-1028 using own 
// private key. Initializtion vector for AES cipher is unsecured.
// Params:
//   data (QVariantMap): map to send encrypted
//   pubKey (QCA::PublicKey): public key of recipient
//   privKey (QCA::PrivateKey): private key of sender
// Returns:
//   (QByteArray): byte array of encrypted map
//      (QByteArray) Message: cipher encrypted byte array of message
//      (QByteArray) Signature: signature of hashed byte array of message
//      (QByteArray) Key: encrypted sym key
//      (QByteArray) iV: initialization vector for cipher
QByteArray encryptMap(QVariantMap data, QCA::PublicKey pubKey, QCA::PrivateKey privKey)
{
    // Initialize encryption tools
    QCA::SymmetricKey symKey(SYMMETRIC_KEY_SIZE);
    QCA::InitializationVector iV(16); 
    if(!QCA::isSupported("aes128-cbc-pkcs7"))
        printf("AES128-CBC not supported!\n");
    
    QCA::Cipher cipher(QString("aes128"), QCA::Cipher::CBC,
            QCA::Cipher::DefaultPadding,
            QCA::Encode,
            symKey, iV);

    // Convert data map to byte array for encryption
    // TODO: No need to convert. Message is already QByteArray.
    QByteArray buffer;
    QDataStream stream(&buffer, QIODevice::WriteOnly);
    stream << data["Message"].toString();

    // Encrypt data
    QCA::SecureArray encryptedMessage = cipher.process(buffer);
    if (!cipher.ok()) {
        qDebug() << "encryptMap: cipher.process(buffer) failed\n";
        data.insert("Success", false);
    }

    // Create signature
    QCA::SecureArray dataDigest = QCA::Hash("sha1").hash(buffer);
    if (!privKey.canSign()) {
        qDebug() << "Can't sign message.";
        data.insert("Success", false);
    }

    QByteArray signature = privKey.signMessage(dataDigest, QCA::EMSA3_MD5);

    // Encrypt symmetric key
    if (!pubKey.canEncrypt()) {
        qDebug() << "Can't encrypt message.";
        data.insert("Success", false);
    }

    QCA::SecureArray encryptedSymKey = pubKey.encrypt(symKey, QCA::EME_PKCS1_OAEP);
    
    // Bundle data and encryption tokens into QVariantMap
    data.insert("Message", encryptedMessage.toByteArray());
    data.insert("Key", encryptedSymKey.toByteArray());
    data.insert("Signature", signature);
    data.insert("iV", iV.toByteArray());
    if (!data.contains("Success"))
        data.insert("Success", true);

    // Convert to byte array
    QByteArray encryptedMap;
    QDataStream encryptedMapStream(&encryptedMap, QIODevice::WriteOnly);
    encryptedMapStream << data;

    return encryptedMap;
}

QVariantMap decryptMap(QByteArray datagram, QCA::PublicKey pubKey, QCA::PrivateKey privKey)
{
    // Convert datagram into QVariantMap
    // TODO: No need to convert, we are passed in a map.
    QDataStream dataStream(&datagram, QIODevice::ReadOnly);
    QVariantMap data;
    dataStream >> data;

    // Do some initial checks
    if (!data["Success"].toBool())
        return data;
    if (!privKey.canDecrypt()) {
        qDebug() << "decryptData: cannot decrypt with private key!\n";
        data.insert("Success", false);
        return data;
    }

    // Decrypt symmetric key
    QCA::SecureArray decryptedSymKey;
    if (0 == privKey.decrypt(data["Key"].toByteArray(), &decryptedSymKey, QCA::EME_PKCS1_OAEP)) {
        qDebug() << "decryptData: decryption of symmetric key failed!\n";
        data.insert("Success", false);
        return data;
    }
    QCA::SymmetricKey symKey(decryptedSymKey);

    // Decrypt data
    QCA::Initializer init;
    QCA::InitializationVector iV(data["iV"].toByteArray());
    QCA::Cipher cipher(QString("aes128"), QCA::Cipher::CBC,
            QCA::Cipher::DefaultPadding,
            QCA::Decode,
            symKey, iV);

    QByteArray decryptedData = cipher.process(data["Message"].toByteArray()).toByteArray();
    if (!cipher.ok()) {
        qDebug() << "decryptData: cipher.update() failed\n";
        data.insert("Success", false);
        return data;
    }
    QCA::SecureArray decryptedDataDigest = QCA::Hash("sha1").hash(decryptedData);

    QDataStream decryptedDataStream(&decryptedData, QIODevice::ReadOnly);
    // TODO: Convert to map instead of string.
    QString decryptedMessage;
    decryptedDataStream >> decryptedMessage;

    // Check signature
    if (!pubKey.verifyMessage(decryptedDataDigest, data["Signature"].toByteArray(), QCA::EMSA3_MD5)) {
        qDebug() << "decryptData: verifyMessage() failed\n";
        data.insert("Success", false);
        return data;
    }

    data.insert("Message", decryptedMessage);
    data.remove("Key");
    data.remove("Signature");
    data.remove("iV");

    return data;
};
