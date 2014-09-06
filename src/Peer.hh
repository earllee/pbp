#ifndef PEER_HH
#define PEER_HH

#include <QObject>
#include <QHostAddress>
#include <QVector>

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
  void connect();
  void connect(QVariantMap);
  void wait();
  QHostAddress host();
  quint16 port();
  bool connected();
public slots:
  void endConnection();
signals:
  void rumor(QVariantMap);
};

#endif
