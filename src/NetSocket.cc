#include <unistd.h>

#include <NetSocket.hh>
#include <QUdpSocket>
#include <QDebug>
#include <Peer>
#include <PeerList>

NetSocket::NetSocket() {
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

NetSocket::~NetSocket() {
  delete peers;
}

bool NetSocket::bind() {
  // Try to bind to each of the range myPortMin..myPortMax in turn.
  for (int p = myPortMin; p <= myPortMax; p++) {
    if (QUdpSocket::bind(p)) {
      qDebug() << "bound to UDP port " << p;
      // make this randomized
      peers = new PeerList();
      connect(peers, SIGNAL(sendMessage(QHostAddress, quint16, QVariantMap)),
	      this, SLOT(sendMessage(QHostAddress, quint16, QVariantMap)));
      peers->setMe(QHostAddress::LocalHost, p);
      for (int port = myPortMin; port <= myPortMax; port++) {
	if (port != p) {
	  peers->add(QHostAddress::LocalHost, port);
	}
      }
      connect(this, SIGNAL(readyRead()),
	      this, SLOT(receiveMessage()));
      return true;
    }
  }

  qDebug() << "Oops, no ports in my default range " << myPortMin
	   << "-" << myPortMax << " available";
  return false;
}


void NetSocket::localMessage(QString text) {
  QVariantMap datagram;
  datagram.insert("ChatText", QVariant(text));
  datagram.insert("Origin", QVariant(peers->myName()));
  datagram.insert("SeqNo", QVariant(peers->mySeqNo()));
  // nullptr means self
  peers->newMessage(peers->myHost(), peers->myPort(), datagram);
}

void NetSocket::sendMessage(QHostAddress host, quint16 port, QVariantMap datagram) {
  QByteArray buffer;
  QDataStream stream(&buffer, QIODevice::WriteOnly);
  stream << datagram;

  writeDatagram(&buffer, host, port);
}

void NetSocket::receiveMessage() {
  while(hasPendingDatagrams()) {
    QByteArray data;
    data.resize(pendingDatagramSize());
    QHostAddress senderAddress;
    quint16 senderPort;
    readDatagram(data.data(), data.size(), &senderAddress, &senderPort);
    QDataStream stream(&data, QIODevice::ReadOnly);
    QVariantMap datagram;
    stream >> datagram;
    
    peers->newMessage(senderAddress, senderPort, datagram);
  }
}
