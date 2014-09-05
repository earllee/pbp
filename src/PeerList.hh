#ifndef PEERLIST_HH
#define PEERLIST_HH

#include <QObject>
#include <Peer.hh>

class PeerList : public QObject
{
  Q_OBJECT

private:
  QVector<Peer> peers;
public:
  PeerList();
  void newMessage(&Peer, QVariantMap);
  Peer *get(QHostAddress, quint16);
  Peer *get(QString);
public slots:
  void relayChat(QString);
signals:
  void chatMessage(QString);
};

#endif
