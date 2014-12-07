#include <unistd.h>

#include <QtCrypto>
#include <QDebug>
#include <Origin.hh>
#include <Peer.hh>
#include <PeerList.hh>
#include <ChatDialog.hh>
#include <NetSocket.hh>
#include <QApplication>
#include <PBP.hh>

void testEncryption() {
  QString txt = "Test message 1 2 3 ...";
  QVariantMap map, msg;
  msg.insert("ChatText", txt);
  QByteArray buffer;
  QDataStream stream(&buffer, QIODevice::WriteOnly);
  stream << msg;
  map.insert("Message", buffer);
  
  QCA::PrivateKey privKeyA = QCA::KeyGenerator().createRSA(1024);
  QCA::PrivateKey privKeyB = QCA::KeyGenerator().createRSA(1024);
  QCA::PublicKey pubKeyA = privKeyA.toPublicKey();
  QCA::PublicKey pubKeyB = privKeyB.toPublicKey();

  encryptMap(map, pubKeyB, privKeyA);

  decryptMap(map, pubKeyA, privKeyB);

  QByteArray messageData = map.value("Message").toByteArray();
  QDataStream stream2(&messageData, QIODevice::ReadOnly);
  QVariantMap message;
  stream2 >> message;

  qDebug() << "Decrypted Map:" << message;

  exit(0);
}

int main(int argc, char **argv) {
  // Initialize QtCrypto
  QCA::Initializer qcainit;

  // Initialize Qt toolkit
  QApplication app(argc,argv);

  bool nofwd = false;
  bool test = false;
  foreach(QString s, app.arguments().mid(1)) {
    if(s == "-noforward")
      nofwd = true;
    if(s == "-test") {
      test = true;
      // testEncryption();
    }
  }

  // Create an initial chat dialog window
  ChatDialog dialog(nofwd);
  // Create a UDP network socket
  NetSocket sock;

  QObject::connect(&dialog, SIGNAL(newMessage(QString, QString)),
		   &sock, SLOT(localMessage(QString, QString)));
  QObject::connect(&sock, SIGNAL(postMessage(QString, QString, QString)),
		   &dialog, SLOT(postMessage(QString, QString, QString)));
  QObject::connect(&dialog, SIGNAL(addPeer(QString)),
		   &sock, SLOT(addPeer(QString)));
  QObject::connect(&sock, SIGNAL(newOrigin(QString)),
		   &dialog, SLOT(newOrigin(QString)));
  QObject::connect(&dialog, SIGNAL(shareFile(QString)),
		   &sock, SLOT(shareFile(QString)));
  QObject::connect(&dialog, SIGNAL(downloadFile(QByteArray, QString, QString)),
		   &sock, SLOT(fileMessage(QByteArray, QString, QString)));
  QObject::connect(&dialog, SIGNAL(search(QString)),
		   &sock, SLOT(searchMessage(QString)));
  QObject::connect(&sock, SIGNAL(searchReply(QByteArray, QString, QString)),
		   &dialog, SLOT(searchReply(QByteArray, QString, QString)));
  QObject::connect(&sock, SIGNAL(receivedBlocklist(QByteArray, qint64)),
		   &dialog, SLOT(receivedBlocklist(QByteArray, qint64)));
  QObject::connect(&sock, SIGNAL(receivedBlock(QByteArray, qint64)),
		   &dialog, SLOT(receivedBlock(QByteArray, qint64)));
  QObject::connect(&sock, SIGNAL(newPeer(QString)),
		   &dialog, SLOT(newPeer(QString)));
  QObject::connect(&dialog, SIGNAL(requestTrust(QString)),
		   &sock, SLOT(requestTrust(QString)));
  QObject::connect(&dialog, SIGNAL(trustApproved(QString)),
		   &sock, SLOT(trustApproved(QString)));
  QObject::connect(&sock, SIGNAL(approveTrust(QString)),
		   &dialog, SLOT(approveTrust(QString)));
  QObject::connect(&sock, SIGNAL(acceptedTrust(QString)),
		   &dialog, SLOT(acceptedTrust(QString)));
  QObject::connect(&sock, SIGNAL(messageable(QString)),
		   &dialog, SLOT(messageable(QString)));

  if (!sock.bind(nofwd))
    exit(1);

  if (test) {
    testEncryption();
    exit(0);
  }


  foreach(QString s, app.arguments().mid(1)) {
    if(s != "-noforward")
      sock.addPeer(s);
  }
  // send an initial routing broadcast
  sock.routeRumor();

  // Enter the Qt main loop; everything else is event driven
  dialog.show();
  return app.exec();  
}

