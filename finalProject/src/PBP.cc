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
QByteArray encryptMap(QVariantMap data, QCA::PublicKey pubKey, QCA::PrivateKey privKey)
{
    // Initialize encryption tools
    QCA::Initializer init;
    QCA::SymmetricKey symKey(SYMMETRIC_KEY_SIZE);
    QCA::InitializationVector iV(16); 
    QCA::Cipher cipher(QString("aes128"), QCA::Cipher::CBC,
            QCA::Cipher::DefaultPadding,
            QCA::Encode,
            symKey, iV);

    // Create new data map to be filled out
    QVariantMap newData;
    newData["success"] = false;

    // Move routing info out of data to be encrypted and into new data map
    QString Type;
    QString Origin;
    QString Dest;
    quint32 Budget;
    quint32 HopLimit;
    quint32 LastIP;
    quint16 LastPort;

    if (data.contains("Type")) {
        Type = data["Type"];
        data.remove("Type");
        newData["Type"] = Type;
    }

    if (data.contains("Origin")) {
        Origin = data["Origin"];
        data.remove("Origin");
        newData["Origin"] = Origin;
    }

    if (data.contains("Dest")) {
        Dest = data["Dest"];
        data.remove("Dest");
        newData["Dest"] = Dest;
    }

    if (data.contains("Budget")) {
        Budget = data["Budget"];
        data.remove("Budget");
        newData["Budget"] = Budget;
    }

    if (data.contains("HopLimit")) {
        HopLimit = data["HopLimit"];
        data.remove("HopLimit");
        newData["HopLimit"] = HopLimit;
    }

    if (data.contains("LastIP")) {
        LastIP = data["LastIP"];
        data.remove("LastIP");
        newData["LastIP"] = LastIP;
    }

    if (data.contains("LastPort")) {
        LastPort = data["LastPort"];
        data.remove("LastPort");
        newData["LastPort"] = LastPort;
    }

    // Convert data map to byte array for encryption
    QByteArray buffer;
    QDataStream stream(&buffer, QIODevice::WriteOnly);
    stream << data;

    // Encrypt data
    QCA::SecureArray encryptedData = cipher.process(buffer);
    if (!cipher.ok()) {
        qDebug() << "encryptMap: cipher.process(buffer) failed\n";
        return newData; // TODO: Figure out what to return on fail
    }

    // Create signature
    QCA::SecureArray dataDigest = QCA::Hash("sha1").hash(buffer);
    QByteArray signature = privKey.signMessage(dataDigest, QCA::EMSA1_SHA1);

    // Encrypt symmetric key
    QCA::SecureArray encryptedSymKey = pubKey.encrypt(symKey, QCA::EME_PKCS1_OAEP);
    
    // Bundle data and encryption tokens into QVariantMap
    newData["encryptedData"] = encryptedData;
    newData["encryptedSymKey"] = encrypytedSymKey;
    newData["signature"] = signature;
    newData["iV"] = iV;
    newData["success"] = true;

    // Convert to byte array
    buffer.clear();
    stream << newData;

    return buffer;
}

QVariantMap decryptMap(QByteArray datagram, QCA::PublicKey pubKey, QCA::PrivateKey privKey)
{
    // Convert datagram into QVariantMap
    QDataStream stream(&datagram, QIODevice::ReadOnly);
    QVariantMap data;
    stream >> data;

    // Do some initial checks
    if (!data["success"])
        return data;
    if (!privKey.canDecrypt()) {
        qDebug() << "decryptData: cannot decrypt with private key!\n";
        data["success"] = false;
        return data;
    }

    // Decrypt symmetric key
    QCA::SecureArray decryptedSymKey;
    if (0 == privKey.decrypt(data["encryptedSymKey"], &decryptedSymKey, QCA::EME_PKCS1_OAEP)) {
        qDebug() << "decryptData: decryption of symmetric key failed!\n";
        data["success"] = false;
        return data;
    }
    QCA::SymmetricKey symKey = decryptedSymKey.data();

    // Decrypt data
    QCA::Initializer init;
    QCA::InitializationVector iV = data["iV"];
    QCA::Cipher cipher(QString("aes128"), QCA::Cipher::CBC,
            QCA::Cipher::DefaultPadding,
            QCA::Decode,
            symKey, iV);

    QCA::SecureArray decryptedData = cipher.process(data["encryptedData"]);
    if (!cipher.ok()) {
        qDebug() << "decryptData: cipher.update() failed\n";
        data["success"] = false;
        return data;
    }

    // Check signature
    if (!pubKey.verifyMessage(decryptedData, data["signature"], QCA::EMSA1_SHA1);) {
        qDebug() << "decryptData: veryMessage() failed\n";
        data["success"] = false;
        return data;
    }

    // TODO: STOPPED 11/14/14 00:42. NEED TO RETURN THE DECRYPTED MAP NOW. GOT ALL THE CONTESNT READY.

};
