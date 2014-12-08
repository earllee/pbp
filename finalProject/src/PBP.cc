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
bool encryptMap(QVariantMap &data, QCA::PublicKey pubKey, QCA::PrivateKey privKey)
{
    // Initialize encryption tools
    QCA::SymmetricKey symKey(SYMMETRIC_KEY_SIZE);
    QCA::InitializationVector iV(16); 
    if(!QCA::isSupported("aes128-cbc-pkcs7")) {
      qDebug() << "AES128-CBC not supported!";
      return false;
    }
    
    QCA::Cipher cipher(QString("aes128"), QCA::Cipher::CBC,
            QCA::Cipher::DefaultPadding,
            QCA::Encode,
            symKey, iV);

    QByteArray buffer = data["Message"].toByteArray();

    // Encrypt data
    QCA::SecureArray encryptedMessage = cipher.process(buffer);
    if (!cipher.ok()) {
        qDebug() << "encryptMap: cipher.process(buffer) failed";
	return false;
    }

    if(!privKey.canSign()) {
      qDebug() << "Error: this kind of key cannot sign";
      return false;
    }
    privKey.startSign( QCA::EMSA3_MD5 );
    privKey.update( buffer ); // just reuse the same message
    QByteArray signature = privKey.signature();

    // Encrypt symmetric key
    if (!pubKey.canEncrypt()) {
        qDebug() << "Public key can't encrypt message.";
	return false;
    }

    QCA::SecureArray encryptedSymKey = pubKey.encrypt(symKey, QCA::EME_PKCS1_OAEP);
    
    // Bundle data and encryption tokens into QVariantMap
    data.insert("Message", encryptedMessage.toByteArray());
    data.insert("Key", encryptedSymKey.toByteArray());
    data.insert("Signature", signature);
    data.insert("iV", iV.toByteArray());
    return true;
}

bool decryptMap(QVariantMap &data, QCA::PublicKey pubKey, QCA::PrivateKey privKey)
{
    // Do some initial checks
    if (!privKey.canDecrypt()) {
        qDebug() << "decryptData: cannot decrypt with private key!";
        return false;
    }

    // Decrypt symmetric key
    QCA::SecureArray decryptedSymKey;
    if (privKey.decrypt(data["Key"].toByteArray(), &decryptedSymKey, QCA::EME_PKCS1_OAEP) == 0) {
        qDebug() << "decryptData: decryption of symmetric key failed!";
        return false;
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
        qDebug() << "decryptData: cipher.update() failed";
	return false;
    }

    if(!pubKey.canVerify()) {
      qDebug() << "Public key cannot verify";
      return false;
    }
    pubKey.startVerify( QCA::EMSA3_MD5 );
    pubKey.update( decryptedData );
    if ( pubKey.validSignature( data["Signature"].toByteArray() ) ) {
      qDebug() << "Bad signature";
      // return false;
    }

    data.insert("Message", decryptedData);
    data.remove("Key");
    data.remove("Signature");
    data.remove("iV");
    return true;
};
