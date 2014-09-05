#ifndef NETSOCKET_HH
#define NETSOCKET_HH

#include <QUdpSocket>
#include <Peer>
#include <PeerList>

class NetSocket : public QUdpSocket
{
  Q_OBJECT

public:
  NetSocket();

  // Bind this socket to a Peerster-specific default port.
  bool bind();
private:
  int myPortMin, myPortMax;
  Peer *me;
  PeerList *peers;

public slots:
  void sendMessage(&Peer, QVariantMap);
  void receiveMessage();

signals:
  void receivedMessage(&Peer, QVariantMap);
};

#endif
