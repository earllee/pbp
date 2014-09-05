#include <unistd.h>

#include <NetSocket.hh>
#include <QUdpSocket>
#include <QDebug>

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
  // Try to bind to each of the range myPortMin..myPortMax in turn.
  for (int p = myPortMin; p <= myPortMax; p++) {
    if (QUdpSocket::bind(p)) {
      qDebug() << "bound to UDP port " << p;
      connect(this, SIGNAL(readyRead()),
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
    this->writeDatagram(buffer, QHostAddress(QHostAddress::LocalHost), p);
  }
}

void NetSocket::receiveMessage()
{
  while(this->hasPendingDatagrams()) {
    QByteArray data;
    data.resize(this->pendingDatagramSize());
    QHostAddress sender;
    quint16 senderPort;
    this->readDatagram(data.data(), data.size(), &sender, &senderPort);
    QDataStream stream(&data, QIODevice::ReadOnly);
    QVariantMap datagram;
    stream >> datagram;
    emit receivedMessage(datagram.value("ChatText").toString());
  }
}
