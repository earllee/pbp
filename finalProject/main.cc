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

int testEncryption() {

  QString msg = "Test message 1 2 3 ...";
  QVariantMap map;
  map.insert("Message", msg);
  
  QCA::PrivateKey *privKeyA = new QCA::PrivateKey();
  QCA::PrivateKey *privKeyB = new QCA::PrivateKey();
  QCA::PublicKey *pubKeyA = new QCA::PublicKey(*privKeyA);
  QCA::PublicKey *pubKeyB = new QCA::PublicKey(*privKeyB);

  qDebug() << "Original Map:\n" << map;

  QByteArray encryptedMap = encryptMap(map, *pubKeyA, *privKeyB);
  QVariantMap decryptedMap = decryptMap(encryptedMap, *pubKeyB, *privKeyA);

  qDebug() << "\n\nDecrypted Map:\n" << decryptedMap;

  exit(0);

}

int main(int argc, char **argv) {
  // Initialize QtCrypto
  QCA::Initializer qcainit;

  // Initialize Qt toolkit
  QApplication app(argc,argv);

  bool nofwd = false;
  foreach(QString s, app.arguments().mid(1)) {
    if(s == "-noforward")
      nofwd = true;
    if(s == "-test")
      testEncryption();
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
  QObject::connect(&dialog, SIGNAL(shareFile(QString, bool)),
		   &sock, SLOT(shareFile(QString, bool)));
  QObject::connect(&dialog, SIGNAL(filePrivate(QString, bool)),
		   &sock, SLOT(filePrivate(QString, bool)));
  QObject::connect(&dialog, SIGNAL(downloadFile(QByteArray, QString, QString)),
		   &sock, SLOT(fileMessage(QByteArray, QString, QString)));
  QObject::connect(&dialog, SIGNAL(search(QString)),
		   &sock, SLOT(searchMessage(QString)));
  QObject::connect(&sock, SIGNAL(searchReply(QByteArray, QString, QString, bool)),
		   &dialog, SLOT(searchReply(QByteArray, QString, QString, bool)));
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
  QObject::connect(&dialog, SIGNAL(requestFriend(QString)),
		   &sock, SLOT(requestFriend(QString)));
  QObject::connect(&dialog, SIGNAL(trustApproved(QString)),
		   &sock, SLOT(trustApproved(QString)));
  QObject::connect(&sock, SIGNAL(approveFriend(QString)),
		   &dialog, SLOT(approveFriend(QString)));
  QObject::connect(&sock, SIGNAL(acceptedFriend(QString)),
		   &dialog, SLOT(acceptedFriend(QString)));

  if (!sock.bind(nofwd))
    exit(1);

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

