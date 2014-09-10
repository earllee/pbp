#ifndef PEER_HH
#define PEER_HH

#include <QObject>
#include <QHostAddress>
#include <QVariant>
#include <QVector>
#include <QTimer>

class Peer : public QObject {
  Q_OBJECT

private:
  QHostAddress *host;
  quint16 port;
  QString domain;
  QVector<QVariantMap> *rumors;
  QTimer *timer;
public:
  Peer(QHostAddress, QString, quint16);
  ~Peer();
  void makeConnection();
  void makeConnection(QVariantMap);
  void wait();
  QHostAddress getHost();
  quint16 getPort();
public slots:
  void endConnection();
signals:
  void rumor(QVariantMap);
};

#endif
