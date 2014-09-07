#ifndef NETSOCKET_HH
#define NETSOCKET_HH

#include <QUdpSocket>
#include <PeerList.hh>

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
  void localMessage(QString);
  void sendMessage(QHostAddress, quint16, QVariantMap);
  void receiveMessage();
  void relayMessage(QString);
signals:
  void postMessage(QString);
};

#endif
