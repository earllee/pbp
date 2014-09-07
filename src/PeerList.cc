#include <unistd.h>

#include <Peer.hh>
#include <QVector>

PeerList::PeerList() {
  peers = new QVector<*Peer>();
  connect(this, SIGNAL(sendMessage(QHostAddress, quint16, QVariantMap)),
	  this, SLOT(sentMessage(QHostAddress, quint16, QVariantMap)));
  origins = new OriginList();
}

PeerList::~PeerList() {
  foreach(Peer *p, peers) {
    delete p;
  }
  delete peers;
  delete me;
}

Peer *PeerList::add(QHostAddress host, quint16 port) {
  Peer *newPeer = new Peer(host, port);
  peers->append(newPeer);
  connect(newPeer, SIGNAL(rumor(QVariantMap)),
	  this, SLOT(rumor(QVariantMap)));
  return newPeer;
}

Peer *PeerList::get(QHostAddress host, quint16 port) {
  foreach(Peer *p, peers) {
    if(p->host == host && p->port == port) {
      return p;
    }
  }
  return nullptr;
}

void PeerList::newMessage(QHostAddress host, quint16 port, QVariantMap datagram) {
  bool isMe = (host == me->host() && port == me->port());
  Peer *sender;
  if(isMe) {
    sender = me;
  } else {
    sender = get(host, port);
    if(sender == nullptr) {
      sender = add(host, port);
    }
  }

  bool isStatus = datagram.value("Want").isValid();
  if(!isMe) {
    sender->connect();
    if(isStatus) {
      if(origins->needMessage(datagram)) {
	emit sendMessage(host, port, origins->status());
      }

      QVariantMap message = origins->nextNeededMessage(datagram);
      if(message.empty()) {
	sender->endConnection();
      } else {
	emit sendMessage(host, port, message);
      }
    }
  }
  if(!isStatus) {
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
  me = new Peer(host, port);
}

QHostAddress PeerList::myHost() {
  return me->host();
}

quint16 PeerList::myPort() {
  return me->port();
}

QString PeerList::myName() {
  return origins->myName();
}

quint32 PeerList::mySeqNo() {
  return origins->mySeqNo();
}

void PeerList::rumor(QVariantMap datagram) {
  QVector<*Peer> available = QVector();
  foreach(Peer *p, peers) {
    if(!p->connected()) {
      available.append(p);
    }
  }
  Peer *recipient = available[qrand() % available.size()];
  recipient->connect(message);
  emit(recipient->host(), recipient->port(), message);
}

void sentMessage(QHostAddress host, quint16 port, QVariantMap datagram) {
  Peer *recipient = get(host, port);
  recipient->wait();
}
