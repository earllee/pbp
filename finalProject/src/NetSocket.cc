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

#define HOPLIMIT 10U
#define INITBUDGET 2U

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
  routeTimer = new QTimer(this);
  routeTimer->setInterval(60000);
  connect(routeTimer, SIGNAL(timeout()),
	  this, SLOT(routeRumor()));
}

NetSocket::~NetSocket() {
  delete peers;
}

bool NetSocket::bind(bool nofwd) {
  // Try to bind to each of the range myPortMin..myPortMax in turn.
  for (int p = myPortMin; p <= myPortMax; p++) {
    if (QUdpSocket::bind(p)) {
      peers = new PeerList(p, nofwd);
      connect(peers, SIGNAL(sendMessage(QHostAddress, quint16, QVariantMap)),
	      this, SLOT(sendMessage(QHostAddress, quint16, QVariantMap)));
      connect(peers, SIGNAL(postMessage(QString, QString, QString)),
	      this, SIGNAL(postMessage(QString, QString, QString)));
      connect(peers, SIGNAL(newOrigin(QString)),
	      this, SIGNAL(newOrigin(QString)));
      connect(peers, SIGNAL(searchReply(QByteArray, QString, QString)),
	      this, SIGNAL(searchReply(QByteArray, QString, QString)));
      connect(peers, SIGNAL(receivedBlocklist(QByteArray, qint64)),
	      this, SIGNAL(receivedBlocklist(QByteArray, qint64)));
      connect(peers, SIGNAL(receivedBlock(QByteArray, qint64)),
	      this, SIGNAL(receivedBlock(QByteArray, qint64)));
      connect(peers, SIGNAL(newPeer(QString)),
	      this, SIGNAL(newPeer(QString)));
      connect(peers, SIGNAL(approveTrust(QString)), 
              this, SIGNAL(approveTrust(QString)));
      connect(peers, SIGNAL(acceptedTrust(QString)), 
              this, SIGNAL(acceptedTrust(QString)));
      connect(peers, SIGNAL(messageable(QString)), 
              this, SIGNAL(messageable(QString)));
                 
      for (int port = myPortMin; port <= myPortMax; port++) {
	if (port != p)
	  peers->add(QHostAddress::LocalHost, port);
      }
      connect(this, SIGNAL(readyRead()),
	      this, SLOT(receiveMessage()));
      routeTimer->start();
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

void NetSocket::localMessage(QString text, QString dest) {
  QVariantMap datagram, message;
  datagram.insert("Origin", QVariant(peers->myName()));
  message.insert("ChatText", QVariant(text));
  if(dest.isEmpty()) {
    message.insert("SeqNo", QVariant(peers->mySeqNo()));
    datagram.insert("Type", QVariant("Rumor"));
  } else {
    datagram.insert("Dest", QVariant(dest));
    datagram.insert("HopLimit", QVariant(HOPLIMIT));
    datagram.insert("Type", QVariant("Private"));
  }
  peers->insertMessage(datagram, message);
  peers->newMessage(peers->myHost(), peers->myPort(), datagram);
}

void NetSocket::fileMessage(QByteArray hash, QString filename, QString dest) {
  peers->startDownload(hash, filename, dest);
  QVariantMap datagram, message;
  datagram.insert("Origin", QVariant(peers->myName()));
  datagram.insert("Dest", QVariant(dest));
  datagram.insert("HopLimit", QVariant(HOPLIMIT));
  datagram.insert("Type", QVariant("BlockRequest"));
  message.insert("BlockRequest", QVariant(hash));
  peers->insertMessage(datagram, message);
  peers->newMessage(peers->myHost(), peers->myPort(), datagram);
}

void NetSocket::searchMessage(QString search) {
  QVariantMap datagram, message;
  datagram.insert("Origin", QVariant(peers->myName()));
  message.insert("Search", QVariant(search));
  datagram.insert("Budget", QVariant(INITBUDGET));
  datagram.insert("Type", QVariant("SearchRequest"));
  peers->insertMessage(datagram, message);
  peers->newMessage(peers->myHost(), peers->myPort(), datagram);
}

void NetSocket::shareFile(QString filename) {
  peers->shareFile(filename);
}

// Construct, send message to given peerStr
void NetSocket::requestTrust(QString peer) {
  peers->requestTrust(peer); 
}

void NetSocket::trustApproved(QString peer) {
  peers->processPendingKeys(peer);
  // send response to requesting peer
  // Let PeerList know that we may process the keys now of given peer
}

void NetSocket::routeRumor() {
  QVariantMap datagram, message;
  datagram.insert("Origin", QVariant(peers->myName()));
  datagram.insert("Type", QVariant("Rumor"));
  message.insert("SeqNo", QVariant(peers->mySeqNo()));
  peers->insertMessage(datagram, message);
  peers->newMessage(peers->myHost(), peers->myPort(), datagram);
}

void NetSocket::sendMessage(QHostAddress host, quint16 port, QVariantMap datagram) {
  QByteArray buffer;
  QDataStream stream(&buffer, QIODevice::WriteOnly);
  stream << datagram;

  writeDatagram(buffer, host, port);
  QVariantMap toPrint = datagram;
  toPrint.insert("Message", QVariant(peers->extractMessage(toPrint)));
  qDebug() << toPrint;
  // qDebug() << QString("[SENT %1:%2] %3").arg(host.toString()).arg(port).arg(stringify(datagram));
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

    QVariantMap toPrint = datagram;
    toPrint.insert("Message", QVariant(peers->extractMessage(toPrint)));
    qDebug() << toPrint;
    // qDebug() << QString("[RECEIVED %1:%2] %3").arg(host.toString()).arg(port).arg(stringify(datagram));
    peers->newMessage(host, port, datagram);
  }
}

QString NetSocket::stringify(QVariantMap datagram) {
  QString str;
  if(datagram.contains("Want")) {
    str.append("Status -> ");
    QVariantMap status = datagram.value("Want").toMap();
    foreach(QString origin, status.keys()) {
      str.append(QString("%1 (%2), ").arg(origin).arg(status.value(origin).toUInt()));
    }
    str = str.left(str.size() - 2);
  } else if (datagram.contains("BlockRequest")) {
    str.append(QString("BlockRequest -> %1 requesting from %2").arg(datagram.value("Origin").toString()).arg(datagram.value("Dest").toString()));
  } else if (datagram.contains("BlockReply")) {
    str.append(QString("BlockReply -> %1 replying to %2").arg(datagram.value("Origin").toString()).arg(datagram.value("Dest").toString()));
  } else if (datagram.contains("Search")) {
    str.append(QString("Search -> %1 searching for %2 (Budget: %3)").arg(datagram.value("Origin").toString()).arg(datagram.value("Search").toString()).arg(datagram.value("Budget").toUInt()));
  } else if (datagram.contains("SearchReply")) {
    str.append(QString("SearchReply -> %1 replying to %2 for %3").arg(datagram.value("Origin").toString()).arg(datagram.value("Dest").toString()).arg(datagram.value("SearchReply").toString()));
  } else if(datagram.contains("Dest")) {
    str.append("Private -> ");
    str.append(QString("%1 to %2 (HopLimit: %3) %4").arg(datagram.value("Origin").toString()).arg(datagram.value("Dest").toString()).arg(datagram.value("HopLimit").toUInt()).arg(datagram.value("ChatText").toString()));
  } else if(datagram.contains("ChatText")) {
    str.append(QString("Rumor -> %1 (SeqNo: %2) %3").arg(datagram.value("Origin").toString()).arg(datagram.value("SeqNo").toUInt()).arg(datagram.value("ChatText").toString()));
  } else {
    str.append(QString("Routing -> %1 (%2)").arg(datagram.value("Origin").toString()).arg(datagram.value("SeqNo").toUInt()));
  }
  return str;
}
