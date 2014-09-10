#include <unistd.h>

#include <QDebug>
#include <Origin.hh>
#include <OriginList.hh>
#include <Peer.hh>
#include <PeerList.hh>
#include <ChatDialog.hh>
#include <NetSocket.hh>
#include <QApplication>

int main(int argc, char **argv) {
  // Initialize Qt toolkit
  QApplication app(argc,argv);

  // Create an initial chat dialog window
  ChatDialog dialog;
  dialog.show();

  // Create a UDP network socket
  NetSocket sock;
  if (!sock.bind()) {
    exit(1);
  }

  QObject::connect(&dialog, SIGNAL(newMessage(QString)),
		   &sock, SLOT(localMessage(QString)));
  QObject::connect(&sock, SIGNAL(postMessage(QString, QString, QColor)),
		   &dialog, SLOT(postMessage(QString, QString, QColor)));
  QObject::connect(&dialog, SIGNAL(addPeer(QString)),
		   &sock, SLOT(addPeer(QString)));

  foreach(QString s, app.arguments().mid(1)) {
    sock.addPeer(s);
  }

  // Enter the Qt main loop; everything else is event driven
  return app.exec();  
}

