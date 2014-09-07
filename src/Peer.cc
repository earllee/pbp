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
