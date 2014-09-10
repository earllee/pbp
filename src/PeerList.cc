#include <unistd.h>

#include <QHostAddress>
#include <QVector>
#include <QTimer>
#include <QDebug>
#include <QHostInfo>
#include <OriginList.hh>
#include <Peer.hh>
#include <PeerList.hh>

PeerList::PeerList() {
  peers = new QVector<Peer*>();
  connect(this, SIGNAL(sendMessage(QHostAddress, quint16, QVariantMap)),
	  this, SLOT(sentMessage(QHostAddress, quint16, QVariantMap)));
  origins = new OriginList();
  connect(origins, SIGNAL(postMessage(QString)),
	  this, SLOT(relayMessage(QString)));
  entropyTimer = new QTimer(this);
  entropyTimer->setInterval(10000);
  connect(entropyTimer, SIGNAL(timeout()),
	  this, SLOT(antiEntropy()));
  entropyTimer->start();
}

PeerList::~PeerList() {
  foreach(Peer *p, *peers) {
    delete p;
  }
  delete peers;
  delete me;
  delete entropyTimer;
}

Peer *PeerList::add(QHostAddress host, QString domain, quint16 port) {
  Peer *newPeer = new Peer(host, domain, port);
  peers->append(newPeer);
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
  foreach(Peer *p, *peers) {
    if(p->getHost() == host && p->getPort() == port) {
      return p;
    }
  }
  return NULL;
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

  bool isStatus = datagram.value("Want").isValid();
  if(isStatus) {
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
    bool isHot = origins->addMessage(datagram);
    if(isHot) {
      rumor(datagram);
    }
    if(!isMe) {
      emit sendMessage(host, port, origins->status());
    }
  }
}

void PeerList::setMe(QHostAddress host, quint16 port) {
  QHostInfo info = QHostInfo::fromName(host.toString());
  if (info.error() == QHostInfo::NoError) {
    me = new Peer(host, "<unknown>", port);
    qDebug() << QString("I am %1:%2 (%3)").arg(host.toString()).arg(port).arg("<unknown>");
  } else {
    me = new Peer(host, info.hostName(), port);
    qDebug() << QString("I am %1:%2 (%3)").arg(host.toString()).arg(port).arg(info.hostName());
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
  int nPeers = peers->size();
  if(nPeers > 0) {
    return peers->value(qrand() % nPeers);
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

void PeerList::relayMessage(QString msg) {
  emit postMessage(msg);
}
