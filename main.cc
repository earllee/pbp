#include <unistd.h>

#include <QDebug>
#include <Origin.hh>
#include <OriginList.hh>
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
  
  // test origin
  Origin a("harro");
  a.addMessage(1, "herro");
  a.addMessage(2, "i'm");
  a.addMessage(3, "marvin");
  Origin b("other");
  b.addMessage(2, "i is");
  b.addMessage(3, "swag");

  // test origin list
  OriginList hello;
  hello.add("harro");
  // test messages are added
  if(hello.addMessage(a.message(1))) {
    hello.addMessage(b.message(2));
    qDebug() << hello.status();
    hello.addMessage(b.message(1));
  }
  // test status updated
  qDebug() << hello.status();

  // test needMessage
  QVariantMap otherStatus = hello.status();
  qDebug() << hello.needMessage(otherStatus);
  QVariantMap otherWant = otherStatus.value("Want").toMap();
  otherWant.insert("harro", QVariant(3));
  otherStatus.insert("Want", QVariant(otherWant));
  qDebug() << hello.needMessage(otherStatus);

  // test nextNeededMessage
  QVariantMap otherStatus2 = hello.status();
  qDebug() << hello.nextNeededMessage(otherStatus2);
  QVariantMap otherWant2 = otherStatus2.value("Want").toMap();
  otherWant2.insert("harro", QVariant(1));
  otherStatus2.insert("Want", QVariant(otherWant2));
  qDebug() << hello.nextNeededMessage(otherStatus2);
}

