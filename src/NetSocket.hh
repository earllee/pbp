#ifndef NETSOCKET_HH
#define NETSOCKET_HH

#include <QUdpSocket>
#include <QColor>
#include <PeerList.hh>

class NetSocket : public QUdpSocket {
  Q_OBJECT

public:
  NetSocket();
  ~NetSocket();
  // Bind this socket to a Peerster-specific default port.
  bool bind();
  
private:
  int myPortMin, myPortMax;
  PeerList *peers;
  QString stringify(QVariantMap);
public slots:
  void localMessage(QString);
  void sendMessage(QHostAddress, quint16, QVariantMap);
  void receiveMessage();
  void relayMessage(QString, QString, QColor);
  void addPeer(QString);
signals:
  void postMessage(QString, QString, QColor);
};

#endif
