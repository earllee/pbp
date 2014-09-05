#ifndef NETSOCKET_HH
#define NETSOCKET_HH

#include <QUdpSocket>

class NetSocket : public QUdpSocket
{
  Q_OBJECT

public:
  NetSocket();

  // Bind this socket to a Peerster-specific default port.
  bool bind();

private:
  int myPortMin, myPortMax;

public slots:
  void sendMessage(QString text);
  void receiveMessage();

signals:
  void receivedMessage(QString text);
};

#endif
