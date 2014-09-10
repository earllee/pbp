#include <unistd.h>

#include <NetSocket.hh>
#include <QUdpSocket>
#include <QDebug>
#include <QColor>
#include <QVariant>
#include <PeerList.hh>
#include <QByteArray>
#include <QHostAddress>
#include <QDataStream>
#include <QHostInfo>

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
      peers = new PeerList();
      connect(peers, SIGNAL(sendMessage(QHostAddress, quint16, QVariantMap)),
	      this, SLOT(sendMessage(QHostAddress, quint16, QVariantMap)));
      peers->setMe(QHostAddress::LocalHost, p);
      for (int port = myPortMin; port <= myPortMax; port++) {
	if (port != p) {
	  peers->add(QHostAddress::LocalHost, port);
	}
      }
      connect(peers, SIGNAL(postMessage(QString, QString, QColor)),
	      this, SLOT(relayMessage(QString, QString, QColor)));
      connect(this, SIGNAL(readyRead()),
	      this, SLOT(receiveMessage()));
      return true;
    }
  }

  qDebug() << "Oops, no ports in my default range " << myPortMin
	   << "-" << myPortMax << " available";
  return false;
}

void NetSocket::addPeer(QString text) {
  int splitter = text.lastIndexOf(':');
  if(splitter == -1 || splitter == text.size() - 1) {
    qDebug() << "Invalid peer" << text;
  } else {
    bool ok;
    quint16 port = text.mid(splitter + 1).toUInt(&ok);
    if(!ok) {
      qDebug() << "Invalid port" << text;
    } else {
      QString hostString = text.left(splitter);
      QHostAddress host(hostString);
      if(!host.isNull()) {
	peers->add(host, port);
      } else {
	peers->add(hostString, port);
      }
    }
  }
}

void NetSocket::localMessage(QString text) {
  QVariantMap datagram;
  datagram.insert("ChatText", QVariant(text));
  datagram.insert("Origin", QVariant(peers->myName()));
  datagram.insert("SeqNo", QVariant(peers->mySeqNo()));
  peers->newMessage(peers->myHost(), peers->myPort(), datagram);
}

void NetSocket::sendMessage(QHostAddress host, quint16 port, QVariantMap datagram) {
  QByteArray buffer;
  QDataStream stream(&buffer, QIODevice::WriteOnly);
  stream << datagram;

  writeDatagram(buffer, host, port);
  qDebug() << QString("Sent message to %1:%2").arg(host.toString()).arg(port) << datagram;
}

void NetSocket::receiveMessage() {
  while(hasPendingDatagrams()) {
    QByteArray data;
    data.resize(pendingDatagramSize());
    QHostAddress host;
    quint16 port;
    readDatagram(data.data(), data.size(), &host, &port);
    QDataStream stream(&data, QIODevice::ReadOnly);
    QVariantMap datagram;
    stream >> datagram;

    qDebug() << QString("Received message from %1:%2").arg(host.toString()).arg(port) << datagram;
    peers->newMessage(host, port, datagram);
  }
}

void NetSocket::relayMessage(QString name, QString msg, QColor color) {
  emit postMessage(name, msg, color);
}
