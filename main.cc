#include <unistd.h>

#include <QDebug>
#include <Origin.hh>
// #include <ChatDialog.hh>
// #include <ChatQTextEdit.hh>
// #include <NetSocket.hh>
// #include <Peer.hh>
// #include <QApplication>

int main(int argc, char **argv)
{
  /*
  // Initialize Qt toolkit
  QApplication app(argc,argv);

  // Create an initial chat dialog window
  ChatDialog dialog;
  dialog.show();

  // Create a UDP network socket
  NetSocket sock;
  if (!sock.bind())
  exit(1);

  QObject::connect(&dialog, SIGNAL(newLocalMessage(QString)),
  &sock, SLOT(localMessage(QString)));
  QObject::connect(&sock, SIGNAL(chatMessage(QString)),
  &dialog, SLOT(postMessage(QString)));

  // Enter the Qt main loop; everything else is event driven
  return app.exec();
  */
  Origin o("harro");
  qDebug() << o.getName() << o.next();
  o.addMessage(2, "i'm");
  o.addMessage(3, "marvin");
  qDebug() << o.message(2);
}

