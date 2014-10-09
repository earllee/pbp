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
  connect(origins, SIGNAL(sendMessage(QHostAddress, quint16, QVariantMap)),
	  this, SIGNAL(sendMessage(QHostAddress, quint16, QVariantMap)));
  connect(origins, SIGNAL(searchReply(QVariantMap)),
	  this, SLOT(gotSearchReply(QVariantMap)));
  connect(origins, SIGNAL(receivedBlocklist(QByteArray, qint64)),
	  this, SIGNAL(receivedBlocklist(QByteArray, qint64)));
  connect(origins, SIGNAL(receivedBlock(QByteArray, qint64)),
	  this, SIGNAL(receivedBlock(QByteArray, qint64)));

  entropyTimer = new QTimer(this);
  entropyTimer->setInterval(10000);
  connect(entropyTimer, SIGNAL(timeout()),
	  this, SLOT(antiEntropy()));
  entropyTimer->start();

  searchTimer = new QTimer(this);
  searchTimer->setInterval(1000);
  connect(searchTimer, SIGNAL(timeout()),
	  this, SLOT(expandSearch())); 
  currentQuery = QVariantMap();
  currentBudget = 0;
  results = new QMap<QByteArray, bool>();

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

void PeerList::startDownload(QString filename, QByteArray meta, QString dest) {
  origins->startDownload(filename, meta, dest);
}

void PeerList::newMessage(QHostAddress host, quint16 port, QVariantMap datagram) {
  Peer *sender = get(host, port);
  if(!sender)
    sender = add(host, port);
  if(sender != me)
    sender->makeConnection();

  if (datagram.contains("Search")) {
    if (nofwd)
      return;
    handleSearch(datagram, sender);
  } else if(datagram.contains("Dest")) {
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

void PeerList::handleSearch(QVariantMap datagram, Peer *sender) {
  QString origin = datagram.value("Origin").toString();
  quint32 budget;
  if (origin == myName()
      && datagram.value("Search").toString() != currentQuery.value("Search").toString()) {
    currentQuery = datagram;
    results->clear();
    currentBudget = datagram.value("Budget").toUInt();
    budget = currentBudget;
    searchTimer->start();
  } else {
    origins->searchMessage(datagram, sender);
    budget = datagram.value("Budget").toUInt() - 1;
    if (budget <= 0)
      return;
  }

  propagateSearch(datagram, budget);
}

void PeerList::gotSearchReply(QVariantMap reply) {
  QByteArray key = reply.value("ID").toByteArray();
  if (reply.value("SearchReply").toString() == currentQuery.value("Search").toString()
      && !results->contains(key)) {
    results->insert(key, true);
    if (results->size() >= 10)
      searchTimer->stop();
    emit searchReply(reply);
  }
}

void PeerList::expandSearch() {
  currentBudget = currentBudget * 2;
  if (currentBudget >= 100) {
    searchTimer->stop();
    return;
  }
  propagateSearch(currentQuery, currentBudget);
}

void PeerList::propagateSearch(QVariantMap datagram, quint32 budget) {
  if (budget >= (quint32) peers->size()) {
    QList<Peer*> recipients = peers->values();
    int nRecipients = recipients.size();
    if (nRecipients == 0)
      return;
    quint32 newBudget = budget / nRecipients;
    quint32 extra = budget % nRecipients;
    datagram.insert("Budget", QVariant(newBudget + 1));
    foreach (Peer *recipient, recipients) {
      if (extra-- == 0) // use up the extra budget
	datagram.insert("Budget", QVariant(newBudget));
      emit sendMessage(recipient->getHost(), recipient->getPort(), datagram);
    }
  } else {
    QList<Peer*> recipients = randoms(budget);
    datagram.insert("Budget", QVariant(1));
    foreach (Peer *recipient, recipients) {
      emit sendMessage(recipient->getHost(), recipient->getPort(), datagram);
    }
  }
}

void PeerList::handlePrivate(QVariantMap datagram, Peer *sender) {
    QString dest = datagram.value("Dest").toString();
    QString origin = datagram.value("Origin").toString();
    if (dest == myName()) {
      origins->privateMessage(datagram, sender);
    } else {
      if(origin != myName()) {
	quint32 hopLimit = datagram.value("HopLimit").toUInt() - 1;
	if(hopLimit <= 0)
	  return;
	datagram.insert("HopLimit", hopLimit);
      } else if (datagram.contains("ChatText")) {
	origins->privateMessage(datagram, sender);
      }
      origins->send(dest, datagram);
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

QList<Peer*> PeerList::randoms(quint32 n) {
  QList<Peer*> values = peers->values();
  int nPeers;
  // assume values.size() is initially larger than n
  while ((quint32) (nPeers = values.size()) > n)
    values.removeAt(qrand() % nPeers); // remove until n peers left
  return values;
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

void PeerList::shareFile(QString filename) {
  origins->shareFile(filename);
}
