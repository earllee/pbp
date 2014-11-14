#include "PBP.hh"

#define SYMMETRIC_KEY_SIZE 16

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
    QCA::SecureArray bufferBlock  = cipher.update(buffer);
    if (!cipher.ok()) {
        qDebug() << "encryptMap: cipher.update(buffer) failed\n";
        return newData; // TODO: Figure out what to return on fail
    }
    QCA::SecureArray finalBlock = cipher.final();
    if (!cipher.ok()) {
        qDebug() << "encryptMap: cipher.final() failed\n";
        return newData; // TODO: Figure out what to return on fail
    }
    // TODO: Unsure if .final() returns complete input data or just last block
    // , for now assuming the latter
    QCA::SecureArray encryptedData = bufferBlock.append(finalBlock);

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
        return newData; // TODO: Figure out what to return on fail
    }

};

QByteArray decryptDatagram(QByteArray secureMsg, QCA::PublicKey pubKey, QCA::PrivateKey privKey) 
{
    // Deserialize the secureMsg and its contents
    QDataStream *secureMsgStream = new QDataStream(&secureMsg, QIODevice::ReadOnly);
    QVariantMap msg;
    *secureMsgStream >> msg;

    QDataStream *secureSymKeyStream = new QDataStream(&(msg["secureSymKey"]), QIODevice::ReadOnly);
    QCA::MemoryRegion secureSymKey;
    *secureSymKeyStream >> secureSymKey;

    QDataStream *secureDatagramStream= new QDataStream(&(msg["secureDatagram"]), QIODevice::ReadOnly);
    QCA::MemoryRegion secureDatagram;
    *secureDatagramStream >> secureDatagram;

    QDataStream *secureDigestStream = new QDataStream(&(msg["secureDigest"]), QIODevice::ReadOnly);
    QCA::MemoryRegion secureDigest;
    *secureDigestStream >> secureDigest;

    QDataStream *iVStream = new QDataStream(&(msg["iV"]), QIODevice::ReadOnly);
    QCA::InitializationVector iV;
    *iVStream >> iV;

    // Decrypt random key with private key (RSA)
    QCA::SecureArray symKey;
    privKey.decrypt(QCA::MemoryRegion(secureSymKey), &symKey, QCA::EME_PKCS1_OAEP);

    // Decrypt message with random key (symmetric)
    QCA::Cipher cipher(QString("aes128"), QCA::Cipher::CBC,
        QCA::Cipher::DefaultPadding, QCA::Decode, symKey, iV);

    QCA::MemoryRegion datagram;
    datagram = cipher.update(secureDatagram);
    datagram = cipher.final();

    // Verify digest with public key of sender (RSA)
    QByteArray myDigest = QCA::Hash("sha1").hash(datagram).toByteArray();
    QByteArray digest = pubKey.encrypt(secureDigest, QCA::EME_PKCS1_OAEP);

    return datagram.toByteArray();
}
