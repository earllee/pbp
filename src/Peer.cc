#include <unistd.h>

#include <Peer.hh>
#include <QVector>

Peer::Peer(QHostAddress h, quint16 p)
{
  host = new QHostAddress(&h);
  port = p;
  connected = false;
  waiting = false;
  initial = nullptr;
  timer = new QTimer(this);
  timer->setInterval(1337);
  timer->setSingleShot(true);
  connect(timer, SIGNAL(timeout()),
	  this, SLOT(endConnection()));
}

Peer::~Peer() {
  delete timer;
  delete host;
}

void Peer::connect() {
  connected = true;
  timer->stop();
}

void Peer::connect(QVariantMap msg) {
  connect();
  initial = new QVariantMap(&msg);
}

void Peer::wait() {
  timer->start();
}

QHostAddress Peer::host() {
  return *host;
}

quint16 Peer::port() {
  return port;
}

bool Peer::connected() {
  return connected;
}

void Peer::endConnection() {
  // keep rumormongering with 50% chance
  if(initial != nullptr && qrand() % 2 == 0) {
    emit rumor(initial);
  }

  delete initial;
  initial = nullptr;
  connected = false;
  timer->stop();
}

/*
void Peer::Add(quint32 seqNo, QString text)
{
  if((int)seqNo > messages.size()) {
    messages.resize(seqNo);
  }
  if(messages.value(seqNo - 1) == "" && seqNo >= need) {
    // set the new message if it doesn't already exist
    messages[seqNo - 1] = text;
    if(seqNo == need) {
      // if the seqNo is the one that is needed then increment need until
      // the first missing message (in case later messages were received
      // first)
      for(; messages.value(need - 1) != ""; need++) {
	// emit all messages that were waiting
	emit chatMessage(name, messages.value(need - 1));
      }
    }
  }
}
*/
