#include <unistd.h>

#include <QHostAddress>
#include <QTimer>
#include <QDebug>
#include <QHostInfo>
#include <QFileInfo>
#include <OriginList.hh>
#include <Peer.hh>
#include <PeerList.hh>

#define HOPLIMIT 10U

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
  connect(origins, SIGNAL(postMessage(QString, QString, QString)),
	  this, SIGNAL(postMessage(QString, QString, QString)));
  connect(origins, SIGNAL(newOrigin(QString)),
	  this, SIGNAL(newOrigin(QString)));
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

void PeerList::startDownload(QByteArray meta, QString filename, QString dest) {
  Origin *o = origins->get(dest);
  if (!o)
    return;
  o->startDownload(meta, filename);
}

void PeerList::newMessage(QHostAddress host, quint16 port, QVariantMap &datagram) {
  Peer *sender = get(host, port);
  if(!sender)
    sender = add(host, port);
  if(sender != me)
    sender->makeConnection();

  QString msgType = datagram.value("Type").toString();
  if (msgType != "Rumor" && nofwd)
    return;

  Origin *from, *to;
  quint32 budget, hopLimit;
  if (datagram.contains("Origin")) {
    QString origin = datagram.value("Origin").toString();
    from = origins->get(origin, sender);
  }
  if (datagram.contains("Dest")) {
    QString dest = datagram.value("Dest").toString();
    to = origins->get(dest, sender);
  }
  if (datagram.contains("Budget"))
    budget = datagram.value("Budget").toUInt();
  if (datagram.contains("HopLimit"))
    hopLimit = datagram.value("HopLimit").toUInt();

  if (msgType == "Rumor")
    handleRumor(datagram, sender, from);
  else if (msgType == "Status")
    handleStatus(datagram, sender);
  else if (msgType == "Private")
    handlePrivate(datagram, from, to, hopLimit);
  else if (msgType == "SearchRequest")
    handleSearchRequest(datagram, from, budget);
  else if (msgType == "SearchReply")
    handleSearchReply(datagram, from, to, hopLimit);
  else if (msgType == "BlockRequest")
    handleBlockRequest(datagram, from, to, hopLimit);
  else if (msgType == "BlockReply")
    handleBlockReply(datagram, from, to, hopLimit);
}

void PeerList::handleSearchRequest(QVariantMap &datagram, Origin *from, quint32 budget) {
  if (from->getName() == myName()) {
    // I'm sending a new search request
    currentQuery = datagram;
    results->clear();
    currentBudget = budget;
    searchTimer->start();
  } else {
    // I'm responding to a search request
    QString query = extractMessage(datagram).value("Search").toString();
    QList<SharedFile*> files = origins->searchFiles(query);
    QVariantList filenames, ids;
    foreach(SharedFile *file, files) {
      filenames.append(QVariant(QFileInfo(file->getFilename()).fileName()));
      ids.append(QVariant(file->getMeta()));
    }
    QVariantMap reply;
    reply.insert("Dest", QVariant(from->getName()));
    reply.insert("Origin", QVariant(myName()));
    reply.insert("HopLimit", QVariant(HOPLIMIT));
    reply.insert("Type", QVariant("SearchReply"));
    QVariantMap replyMessage;
    replyMessage.insert("SearchReply", QVariant(query));
    replyMessage.insert("MatchNames", QVariant(filenames));
    replyMessage.insert("MatchIDs", QVariant(ids));
    insertMessage(reply, replyMessage);

    Peer *hop = from->getHop();
    emit sendMessage(hop->getHost(), hop->getPort(), reply);

    // decide whether to propagate
    budget = budget - 1;
    if (budget <= 0)
      return;
  }

  propagateSearch(datagram, budget);
}

void PeerList::handleSearchReply(QVariantMap &datagram, Origin *from, Origin *dest, quint32 hopLimit) {
  if (dest->getName() == myName()) {
    QVariantMap message = extractMessage(datagram);
    QVariantList filenames = message.value("MatchNames").toList();
    QVariantList ids = message.value("MatchIDs").toList();
    int len = filenames.size();

    for (int i = 0; i < len; i++) {
      QByteArray key = ids.at(i).toByteArray();
      if (message.value("SearchReply").toString() == currentQuery.value("Search").toString()
	  && !results->contains(key)) {
	results->insert(key, true);
	if (results->size() >= 10)
	  searchTimer->stop();
	emit searchReply(ids.at(i).toByteArray(), filenames.at(i).toString(), from->getName());
      }
    }
  } else {
    forwardMessage(datagram, dest, hopLimit);
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

void PeerList::propagateSearch(QVariantMap &datagram, quint32 budget) {
  if (budget >= (quint32) peers->size()) {
    // every peer gets the message
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
    // budget only reaches some peers
    QList<Peer*> recipients = randoms(budget);
    datagram.insert("Budget", QVariant(1));
    foreach (Peer *recipient, recipients) {
      emit sendMessage(recipient->getHost(), recipient->getPort(), datagram);
    }
  }
}

void PeerList::handlePrivate(QVariantMap &datagram, Origin *from, Origin *to, quint32 hopLimit) {
  QString chatText, fromName, toName;
  fromName = from->getName();
  toName = to->getName();
  if (fromName == myName() || toName == myName()) {
    chatText = extractMessage(datagram).value("ChatText").toString();
  }

  if (toName == myName()) {
    // post message sent to me
    emit postMessage(fromName, chatText, fromName);
  } else if (fromName == myName()) {
    // post message sent by me and forward
    emit postMessage(fromName, chatText, toName);
    forwardMessage(datagram, to, hopLimit + 1);
  } else {
    forwardMessage(datagram, to, hopLimit);
  }
}

void PeerList::forwardMessage(QVariantMap &datagram, Origin *to, quint32 hopLimit) {
  hopLimit--;
  if (hopLimit <= 0)
    return;
  datagram.insert("HopLimit", hopLimit);

  Peer *hop = to->getHop();
  emit sendMessage(hop->getHost(), hop->getPort(), datagram);
}

void PeerList::handleStatus(QVariantMap &datagram, Peer *sender) {
  QVariantMap reply;
  QVariantMap message = extractMessage(datagram);
  QVariantMap needed = origins->nextNeededMessage(message);
  if (needed.empty()) {
    // send status back if there are messages missing
    if (origins->needMessage(message))
      emit sendMessage(sender->getHost(), sender->getPort(), constructStatus());
    else
      sender->endConnection();
  } else {
    reply.insert("Type", QVariant("Rumor"));
    reply.insert("Origin", needed.value("Origin"));
    insertMessage(reply, needed);
    emit sendMessage(sender->getHost(), sender->getPort(), reply);
  }
}

void PeerList::handleRumor(QVariantMap &datagram, Peer *sender, Origin *from) {
  Peer *last = sender;
  bool direct = true;
  if (datagram.contains("LastIP")) {
    direct = false;
    QHostAddress lastHost(datagram.value("LastIP").toUInt());
    quint16 lastPort = datagram.value("LastPort").toUInt();
    if (!lastHost.isNull()) {
      last = get(lastHost, lastPort);
      if (!last)
	last = add(lastHost, lastPort);
    }
  }

  // is rumor
  QVariantMap message = extractMessage(datagram);
  if (from->addMessage(message.value("SeqNo").toUInt(), message, last, direct)) {
    // add message returns whether or not the message is new (i.e., hot)
    if (sender != me) {
      datagram.insert("LastIP", QVariant(sender->getHost().toIPv4Address()));
      datagram.insert("LastPort", QVariant(sender->getPort()));
    }
    // broadcast to everyone if it's a routing message
    rumor(datagram, !message.contains("ChatText"));
  }
  if(sender != me && !nofwd)
    emit sendMessage(sender->getHost(), sender->getPort(), constructStatus());
}

void PeerList::handleBlockRequest(QVariantMap &datagram, Origin *from, Origin *to, quint32 hopLimit) {
  if (to->getName() == myName()) {
    QVariantMap message = from->blockRequest(extractMessage(datagram), origins->get(myName()));
    if (message.empty())
      return;
    QVariantMap reply;
    reply.insert("Dest", from->getName());
    reply.insert("Origin", myName());
    reply.insert("HopLimit", QVariant(HOPLIMIT));
    reply.insert("Type", QVariant("BlockReply"));
    insertMessage(reply, message);
    forwardMessage(reply, from, HOPLIMIT + 1);
  } else if (from->getName() == myName()) {
    forwardMessage(datagram, to, hopLimit + 1);
  } else {
    forwardMessage(datagram, to, hopLimit);
  }
}

void PeerList::handleBlockReply(QVariantMap &datagram, Origin *from, Origin *to, quint32 hopLimit) {
  if (to->getName() == myName()) {
    QVariantMap message = from->blockReply(extractMessage(datagram));
    if (message.empty())
      return;
    QVariantMap request;
    request.insert("Dest", from->getName());
    request.insert("Origin", myName());
    request.insert("HopLimit", QVariant(HOPLIMIT));
    request.insert("Type", QVariant("BlockRequest"));
    insertMessage(request, message);
    forwardMessage(request, from, HOPLIMIT + 1);
  } else if (from->getName() == myName()) {
    forwardMessage(datagram, to, hopLimit + 1);
  } else {
    forwardMessage(datagram, to, hopLimit);
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
    emit sendMessage(recipient->getHost(), recipient->getPort(), constructStatus());
  }
}

void PeerList::sentMessage(QHostAddress host, quint16 port) {
  Peer *recipient = get(host, port);
  recipient->wait();
}

void PeerList::shareFile(QString filename) {
  origins->shareFile(filename);
}

QVariantMap PeerList::extractMessage(QVariantMap &datagram) {
  QByteArray messageData = datagram.value("Message").toByteArray();
  QDataStream stream(&messageData, QIODevice::ReadOnly);
  QVariantMap message;
  stream >> message;
  return message;
}

void PeerList::insertMessage(QVariantMap &datagram, QVariantMap message) {
  QByteArray buffer;
  QDataStream stream(&buffer, QIODevice::WriteOnly);
  stream << message;
  datagram.insert("Message", QVariant(buffer));
}

QVariantMap PeerList::constructStatus() {
  QVariantMap datagram;
  insertMessage(datagram, origins->status());
  datagram.insert("Type", QVariant("Status"));
  return datagram;
}
