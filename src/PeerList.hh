#ifndef PEERLIST_HH
#define PEERLIST_HH

#include <QObject>
#include <QHostAddress>
#include <QVector>
#include <QTimer>
#include <Peer.hh>
#include <OriginList.hh>

class PeerList : public QObject {
  Q_OBJECT

private:
  Peer *me;
  QVector<Peer*> *peers;
  OriginList *origins;
  QTimer *entropyTimer;
  Peer *get(QHostAddress, quint16);
  Peer *random();
public:
  PeerList();
  ~PeerList();
  Peer *add(QHostAddress, quint16);
  void newMessage(QHostAddress, quint16, QVariantMap);
  void setMe(QHostAddress, quint16);
  QHostAddress myHost();
  quint16 myPort();
  QString myName();
  quint32 mySeqNo();
public slots:
  void rumor(QVariantMap);
  void sentMessage(QHostAddress, quint16, QVariantMap);
  void relayMessage(QString);
  void antiEntropy();
signals:
  void sendMessage(QHostAddress, quint16, QVariantMap);
  void postMessage(QString);
};

#endif