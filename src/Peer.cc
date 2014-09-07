#include <unistd.h>

#include <Peer.hh>
#include <QHostAddress>
#include <QVariant>
#include <QTimer>

Peer::Peer(QHostAddress h, quint16 p) {
  host = new QHostAddress(h);
  port = p;
  connected = false;
  initial = NULL;
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

void Peer::makeConnection() {
  connected = true;
  timer->stop();
}

void Peer::makeConnection(QVariantMap msg) {
  makeConnection();
  initial = new QVariantMap(msg);
}

void Peer::wait() {
  timer->start();
}

QHostAddress Peer::getHost() {
  return *host;
}

quint16 Peer::getPort() {
  return port;
}

bool Peer::isConnected() {
  return connected;
}

void Peer::endConnection() {
  // keep rumormongering with 50% chance
  if(initial != NULL && qrand() % 2 == 0) {
    emit rumor(*initial);
  }

  delete initial;
  initial = NULL;
  connected = false;
  timer->stop();
}
