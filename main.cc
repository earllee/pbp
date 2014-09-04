
#include <unistd.h>

#include <QVBoxLayout>
#include <QApplication>
#include <QDebug>
#include <QKeyEvent>

#include "main.hh"

bool ChatQTextEdit::eventFilter(QObject *obj, QEvent *event)
{
  if (event->type() == QEvent::KeyPress) {
    QKeyEvent *keyEvent = static_cast<QKeyEvent*>(event);
    switch(keyEvent->key()) {
    case Qt::Key_Enter:
    case Qt::Key_Return:
      emit returnPressed();
      return true;
    }
  }
  return false;
}

ChatDialog::ChatDialog()
{
	setWindowTitle("Peerster");

	// Read-only text box where we display messages from everyone.
	// This widget expands both horizontally and vertically.
	textview = new QTextEdit(this);
	textview->setReadOnly(true);

	// Small text-entry box the user can enter messages.
	// This widget normally expands only horizontally,
	// leaving extra vertical space for the textview widget.
	//
	// You might change this into a read/write QTextEdit,
	// so that the user can easily enter multi-line messages.
	textline = new ChatQTextEdit(this);
	textline->setMaximumHeight(3 * textline->font().pointSize() + 32);
	textline->installEventFilter(textline);

	// Lay out the widgets to appear in the main window.
	// For Qt widget and layout concepts see:
	// http://doc.qt.nokia.com/4.7-snapshot/widgets-and-layouts.html
	QVBoxLayout *layout = new QVBoxLayout();
	layout->addWidget(textview);
	layout->addWidget(textline);
	setLayout(layout);

	// set focus on text input
	textline->setFocus();

	// Register a callback on the textline's returnPressed signal
	// so that we can send the message entered by the user.
	connect(textline, SIGNAL(returnPressed()),
		this, SLOT(gotReturnPressed()));
}

void ChatDialog::gotReturnPressed()
{
	// Initially, just echo the string locally.
	// Insert some networking code here...
	qDebug() << "FIX: send message to other peers: " << textline->toPlainText();
	emit newMessage(textline->toPlainText());

	// Clear the textline to get ready for the next input message.
	textline->clear();
}

void ChatDialog::postMessage(QString text)
{
  textview->append(text);
}

NetSocket::NetSocket()
{
	// Pick a range of four UDP ports to try to allocate by default,
	// computed based on my Unix user ID.
	// This makes it trivial for up to four Peerster instances per user
	// to find each other on the same host,
	// barring UDP port conflicts with other applications
	// (which are quite possible).
	// We use the range from 32768 to 49151 for this purpose.
	myPortMin = 32768 + (getuid() % 4096)*4;
	myPortMax = myPortMin + 3;
}

bool NetSocket::bind()
{
  udp = new QUdpSocket(this);

  // Try to bind to each of the range myPortMin..myPortMax in turn.
  for (int p = myPortMin; p <= myPortMax; p++) {
    if (udp->bind(p)) {
      qDebug() << "bound to UDP port " << p;
      connect(udp, SIGNAL(readyRead()),
	      this, SLOT(receiveMessage()));
      return true;
    }
  }

  qDebug() << "Oops, no ports in my default range " << myPortMin
	   << "-" << myPortMax << " available";
  return false;
}

void NetSocket::sendMessage(QString text)
{
  QVariantMap *datagram = new QVariantMap;
  datagram->insert("ChatText", QVariant(text));
  QByteArray buffer;
  QDataStream stream(&buffer, QIODevice::WriteOnly);
  stream << *datagram;
  for (int p = myPortMin; p <= myPortMax; p++) {
    udp->writeDatagram(buffer, QHostAddress(QHostAddress::LocalHost), p);
  }
}

void NetSocket::receiveMessage()
{
  while(udp->hasPendingDatagrams()) {
    QByteArray data;
    data.resize(udp->pendingDatagramSize());
    QHostAddress sender;
    quint16 senderPort;
    udp->readDatagram(data.data(), data.size(), &sender, &senderPort);
    QDataStream stream(&data, QIODevice::ReadOnly);
    QVariantMap datagram;
    stream >> datagram;
    emit receivedMessage(datagram.value("ChatText").toString());
  }
}

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

