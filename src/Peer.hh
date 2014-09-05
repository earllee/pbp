#ifndef PEER_HH
#define PEER_HH

#include <QObject>
#include <QHostAddress>
#include <QVector>

class Peer : public QObject
{
  Q_OBJECT

private:
  QString name;
  QHostAddress host;
  quint16 port;
  QVector<QString> messages;
  quint32 need;
  PeerConnection connection;
public:
  Peer(QString, QHostAddress, quint16);
  void newMessage(QVariantMap, QVariantMap);
  void add(quint32, QString);
  QString get(quint32);
  quint32 next();
  QString name();
  void endConnection();
public slots:
  void checkConnection();
signals:
  void chatMessage(QString);
  void sendMessage(QString, quint32);
  void sendStatus(QString);
};

#endif
