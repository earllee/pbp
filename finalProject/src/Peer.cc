#include <unistd.h>

#include <Peer.hh>
#include <QHostAddress>
#include <QVariant>
#include <QVector>
#include <QTimer>

Peer::Peer(QHostAddress h, QString d, quint16 p) {
  host = new QHostAddress(h);
  domain = d;
  port = p;
  rumors = new QVector<QVariantMap>;
  timer = new QTimer(this);
  timer->setInterval(1000);
  timer->setSingleShot(true);
  connect(timer, SIGNAL(timeout()),
	  this, SLOT(endConnection()));
}

Peer::~Peer() {
  delete timer;
  delete host;
}

void Peer::makeConnection() {
  timer->stop();
}

void Peer::makeConnection(QVariantMap msg) {
  makeConnection();
  rumors->append(msg);
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

void Peer::endConnection() {
  foreach(QVariantMap msg, *rumors) {
    // keep rumormongering with 50% chance
    if (qrand() % 2)
      emit rumor(msg);
  }
  rumors->clear();
  timer->stop();
}
