#ifndef NETSOCKET_HH
#define NETSOCKET_HH

#include <QUdpSocket>
#include <PeerList>

class NetSocket : public QUdpSocket
{
  Q_OBJECT

public:
  NetSocket();
  ~NetSocket();
  // Bind this socket to a Peerster-specific default port.
  bool bind();
  
private:
  int myPortMin, myPortMax;
  PeerList *peers;

public slots:
  void sendMessage(QHostAddress, quint16, QVariantMap);
  void receiveMessage();
};

#endif
