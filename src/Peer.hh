#ifndef PEER_HH
#define PEER_HH

#include <QObject>
#include <QHostAddress>
#include <QVariant>
#include <QTimer>

class Peer : public QObject
{
  Q_OBJECT

private:
  QHostAddress *host;
  quint16 port;
  bool connected;
  QVariantMap *initial;
  QTimer *timer;
public:
  Peer(QHostAddress, quint16);
  ~Peer();
  void makeConnection();
  void makeConnection(QVariantMap);
  void wait();
  QHostAddress getHost();
  quint16 getPort();
  bool isConnected();
public slots:
  void endConnection();
signals:
  void rumor(QVariantMap);
};

#endif
