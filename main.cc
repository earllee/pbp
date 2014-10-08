#include <unistd.h>

#include <QtCrypto>
#include <QDebug>
#include <Origin.hh>
#include <OriginList.hh>
#include <Peer.hh>
#include <PeerList.hh>
#include <ChatDialog.hh>
#include <NetSocket.hh>
#include <QApplication>

int main(int argc, char **argv) {
  // Initialize QtCrypto
  QCA::Initializer qcainit;

  // Initialize Qt toolkit
  QApplication app(argc,argv);

  bool nofwd = false;
  foreach(QString s, app.arguments().mid(1)) {
    if(s == "-noforward")
      nofwd = true;
  }

  // Create an initial chat dialog window
  ChatDialog dialog(nofwd);
  dialog.show();

  // Create a UDP network socket
  NetSocket sock;
  if (!sock.bind(nofwd))
    exit(1);

  QObject::connect(&dialog, SIGNAL(newMessage(QString, QString)),
		   &sock, SLOT(localMessage(QString, QString)));
  QObject::connect(&sock, SIGNAL(postMessage(QString, QString, QColor, QString)),
		   &dialog, SLOT(postMessage(QString, QString, QColor, QString)));
  QObject::connect(&dialog, SIGNAL(addPeer(QString)),
		   &sock, SLOT(addPeer(QString)));
  QObject::connect(&sock, SIGNAL(newOrigin(QString)),
		   &dialog, SLOT(newOrigin(QString)));
  QObject::connect(&dialog, SIGNAL(shareFile(QString)),
		   &sock, SLOT(shareFile(QString)));
  QObject::connect(&dialog, SIGNAL(downloadFile(QString, QByteArray, QString)),
		   &sock, SLOT(fileMessage(QString, QByteArray, QString)));

  foreach(QString s, app.arguments().mid(1)) {
    if(s != "-noforward")
      sock.addPeer(s);
  }

  sock.routeRumor();

  // Enter the Qt main loop; everything else is event driven
  return app.exec();  
}

