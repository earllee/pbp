#include <unistd.h>

#include <QHostAddress>
#include <QTimer>
#include <QDebug>
#include <QHostInfo>
#include <QColor>
#include <OriginList.hh>
#include <Peer.hh>
#include <PeerList.hh>

PeerList::PeerList(quint16 port, bool nf) {
  peers = new QMap<QString, Peer*>();
  connect(this, SIGNAL(sendMessage(QHostAddress, quint16, QVariantMap)),
	  this, SLOT(sentMessage(QHostAddress, quint16)));

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

  nofwd = nf;
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
  if(host == me->getHost() && port == me->getPort())
    return me;
  else
    return peers->value(QString("%1:%2").arg(host.toString()).arg(port));
}

void PeerList::newMessage(QHostAddress host, quint16 port, QVariantMap datagram) {
  Peer *sender = get(host, port);
  if(!sender)
    sender = add(host, port);
  if(sender != me)
    sender->makeConnection();

  if(datagram.contains("Dest")) {
    if (nofwd)
      return;
    handlePrivate(datagram, sender);
  } else if(datagram.contains("Want")) {
    if (nofwd)
      return;
    handleStatus(datagram, sender);
  } else {
    handleRumor(datagram, sender);
  }
}

void PeerList::handlePrivate(QVariantMap datagram, Peer *sender) {
    QString dest = datagram.value("Dest").toString();
    QString origin = datagram.value("Origin").toString();
    if(dest == myName()) {
      origins->privateMessage(datagram, sender);
    } else {
      bool send = true;
      if(origin == myName()) {
	origins->privateMessage(datagram, sender);
      } else {
	quint32 hopLimit = datagram.value("HopLimit").toUInt();
	if(hopLimit > 0)
	  datagram.insert("HopLimit", hopLimit - 1);
	else
	  send = false;
      }
      if(send) {
	Peer *hop = origins->nextHop(dest);
	if(hop)
	  emit sendMessage(hop->getHost(), hop->getPort(), datagram);
      }
    }
}

void PeerList::handleStatus(QVariantMap datagram, Peer *sender) {
  QVariantMap message = origins->nextNeededMessage(datagram);
  if(message.empty()) {
    // send status back if there are messages missing
    if(origins->needMessage(datagram))
      emit sendMessage(sender->getHost(), sender->getPort(), origins->status());
    else
      sender->endConnection();
  } else {
    emit sendMessage(sender->getHost(), sender->getPort(), message);
  }
}

void PeerList::handleRumor(QVariantMap datagram, Peer *sender) {
    Peer *last = sender;
    bool direct = true;
    if(datagram.contains("LastIP")) {
      direct = false;
      QHostAddress lastHost(datagram.value("LastIP").toUInt());
      quint16 lastPort = datagram.value("LastPort").toUInt();
      if(!lastHost.isNull()) {
	last = get(lastHost, lastPort);
	if (!last)
	  last = add(lastHost, lastPort);
      }
    }
    // is rumor
    if(origins->addMessage(datagram, last, direct)) {
      // add message returns whether or not the message is new (i.e., hot)
      if(sender != me) {
	datagram.insert("LastIP", QVariant(sender->getHost().toIPv4Address()));
	datagram.insert("LastPort", QVariant(sender->getPort()));
      }
      // broadcast to everyone if it's a routing message
      rumor(datagram, !datagram.contains("ChatText"));
    }
    if(sender != me && !nofwd)
      emit sendMessage(sender->getHost(), sender->getPort(), origins->status());
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

void PeerList::rumor(QVariantMap datagram, bool broadcast) {
  if (nofwd && datagram.contains("ChatText"))
    return;
  if(broadcast) {
    foreach(Peer *recipient, peers->values()) {
      recipient->makeConnection(datagram);
      emit sendMessage(recipient->getHost(), recipient->getPort(), datagram);
    }
  } else {
    Peer *recipient = random();
    if(recipient != NULL) {
      recipient->makeConnection(datagram);
      emit sendMessage(recipient->getHost(), recipient->getPort(), datagram);
    }
  }
}

void PeerList::antiEntropy() {
  if (nofwd)
    return;
  Peer *recipient = random();
  if(recipient != NULL) {
    recipient->makeConnection();
    emit sendMessage(recipient->getHost(), recipient->getPort(), origins->status());
  }
}

void PeerList::sentMessage(QHostAddress host, quint16 port) {
  Peer *recipient = get(host, port);
  recipient->wait();
}
