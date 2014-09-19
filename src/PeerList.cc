#include <unistd.h>

#include <QHostAddress>
#include <QTimer>
#include <QDebug>
#include <QHostInfo>
#include <QColor>
#include <OriginList.hh>
#include <Peer.hh>
#include <PeerList.hh>

PeerList::PeerList(quint16 port) {
  peers = new QMap<QString, Peer*>();
  connect(this, SIGNAL(sendMessage(QHostAddress, quint16, QVariantMap)),
	  this, SLOT(sentMessage(QHostAddress, quint16, QVariantMap)));

  QHostAddress host = QHostAddress::LocalHost;
  QHostInfo info = QHostInfo::fromName(host.toString());
  if (info.error() == QHostInfo::NoError) {
    me = new Peer(host, "<unknown>", port);
    qDebug() << QString("I am %1:%2 (%3)").arg(host.toString()).arg(port).arg("<unknown>");
  } else {
    me = new Peer(host, info.hostName(), port);
    qDebug() << QString("I am %1:%2 (%3)").arg(host.toString()).arg(port).arg(info.hostName());
  }

  origins = new OriginList(me);
  connect(origins, SIGNAL(postMessage(QString, QString, QColor, QString)),
	  this, SIGNAL(postMessage(QString, QString, QColor, QString)));
  connect(origins, SIGNAL(newOrigin(QString)),
	  this, SIGNAL(newOrigin(QString)));

  entropyTimer = new QTimer(this);
  entropyTimer->setInterval(10000);
  connect(entropyTimer, SIGNAL(timeout()),
	  this, SLOT(antiEntropy()));
  entropyTimer->start();
}

PeerList::~PeerList() {
  foreach(Peer *p, peers->values()) {
    delete p;
  }
  delete peers;
  delete me;
  delete entropyTimer;
}

Peer *PeerList::add(QHostAddress host, QString domain, quint16 port) {
  Peer *newPeer = new Peer(host, domain, port);
  peers->insert(QString("%1:%2").arg(host.toString()).arg(port), newPeer);
  connect(newPeer, SIGNAL(rumor(QVariantMap)),
	  this, SLOT(rumor(QVariantMap)));
  qDebug() << QString("Peer added %1:%2 (%3)").arg(host.toString()).arg(port).arg(domain);
  return newPeer;
}

Peer *PeerList::add(QHostAddress host, quint16 port) {
  QHostInfo info = QHostInfo::fromName(host.toString());
  if (info.error() == QHostInfo::NoError) {
    return add(host, "<unknown>", port);
  } else {
    return add(host, info.hostName(), port);
  }
}

Peer *PeerList::add(QString domain, quint16 port) {
  QHostInfo info = QHostInfo::fromName(domain);
  if (info.error() == QHostInfo::NoError && !info.addresses().isEmpty()) {
    return add(info.addresses().first(), domain, port);
  } else {
    qDebug() << "Invalid host" << domain;
    return NULL;
  }
}

Peer *PeerList::get(QHostAddress host, quint16 port) {
  return peers->value(QString("%1:%2").arg(host.toString()).arg(port));
}

void PeerList::newMessage(QHostAddress host, quint16 port, QVariantMap datagram) {
  bool isMe = (host == me->getHost() && port == me->getPort());
  Peer *sender;
  if(isMe) {
    sender = me;
  } else {
    sender = get(host, port);
    if(sender == NULL) {
      sender = add(host, port);
    }
    sender->makeConnection();
  }

  if(datagram.contains("Dest")) {
    // is private
    QString dest = datagram.value("Dest").toString();
    QString origin = datagram.value("Origin").toString();
    if(dest == myName()) {
      origins->privateMessage(datagram, sender);
    } else {
      if(origin == myName()) {
	origins->privateMessage(datagram, sender);
      } else {
	quint32 hopLimit = datagram.value("HopLimit").toUInt();
	if(--hopLimit > 0)
	  datagram.insert("HopLimit", hopLimit);
      }
      Peer *hop = origins->nextHop(dest);
      if(hop)
	emit sendMessage(hop->getHost(), hop->getPort(), datagram);
    }
    return;
  } else if(datagram.contains("Want")) {
    // is status
    QVariantMap message = origins->nextNeededMessage(datagram);
    if(message.empty()) {
      // send status back if there are messages missing
      if(origins->needMessage(datagram)) {
	emit sendMessage(host, port, origins->status());
      } else {
	sender->endConnection();
      }
    } else {
      emit sendMessage(host, port, message);
    }
  } else {
    // is rumor
    bool isHot = origins->addMessage(datagram, sender);
    if(isHot) {
      rumor(datagram);
    }
    if(!isMe) {
      emit sendMessage(host, port, origins->status());
    }
  }
}

QHostAddress PeerList::myHost() {
  return me->getHost();
}

quint16 PeerList::myPort() {
  return me->getPort();
}

QString PeerList::myName() {
  return origins->myName();
}

quint32 PeerList::mySeqNo() {
  return origins->mySeqNo();
}

Peer *PeerList::random() {
  QList<Peer*> values = peers->values();
  int nPeers = values.size();
  if(nPeers > 0) {
    return values.value(qrand() % nPeers);
  } else {
    return NULL;
  }
}

void PeerList::rumor(QVariantMap datagram) {
  Peer *recipient = random();
  if(recipient != NULL) {
    recipient->makeConnection(datagram);
    emit sendMessage(recipient->getHost(), recipient->getPort(), datagram);
  }
}

void PeerList::antiEntropy() {
  Peer *recipient = random();
  if(recipient != NULL) {
    recipient->makeConnection();
    emit sendMessage(recipient->getHost(), recipient->getPort(), origins->status());
  }
}

void PeerList::sentMessage(QHostAddress host, quint16 port, QVariantMap datagram) {
  Peer *recipient = get(host, port);
  recipient->wait();
}
