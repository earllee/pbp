#include <unistd.h>

#include <ChatDialog.hh>
#include <ChatQTextEdit.hh>
#include <NetSocket.hh>
#include <QApplication>

int main(int argc, char **argv)
{
	// Initialize Qt toolkit
	QApplication app(argc,argv);

	// Create an initial chat dialog window
	ChatDialog dialog;
	dialog.show();

	// Create a UDP network socket
	NetSocket sock;
	if (!sock.bind())
	  exit(1);

	QObject::connect(&dialog, SIGNAL(newMessage(QString)),
			 &sock, SLOT(sendMessage(QString)));
	QObject::connect(&sock, SIGNAL(receivedMessage(QString)),
			 &dialog, SLOT(postMessage(QString)));

	// Enter the Qt main loop; everything else is event driven
	return app.exec();
}

