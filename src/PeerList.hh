#ifndef PEERLIST_HH
#define PEERLIST_HH

#include <QObject>
#include <QHostAddress>
#include <Peer.hh>

class PeerList : public QObject
{
  Q_OBJECT

private:
  Peer *me;
  QVector<*Peer> *peers;
  OriginList *origins;
  Peer *get(QHostAddress, quint16);
public:
  PeerList();
  ~PeerList();
  Peer *add(QHostAddress, quint16);
  void newMessage(QHostAddress, quint16, QVariantMap);
  void setMe(QHostAddress, quint16);
  QHostAddress myHost();
  quint16 myPort();
  QString myOrigin();
  quint32 mySeqNo();
public slots:
  void rumor(QVariantMap);
  void sentMessage(QHostAddress, quint16, QVariantMap);
signals:
  void sendMessage(QHostAddress, quint16, QVariantMap);
};

#endif
