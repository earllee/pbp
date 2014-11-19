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
  bool bind(bool nofwd = false);
  
private:
  int myPortMin, myPortMax;
  PeerList *peers;
  QString stringify(QVariantMap);
  QTimer *routeTimer;
public slots:
  void searchMessage(QString);
  void localMessage(QString, QString);
  void fileMessage(QByteArray, QString, QString);
  void sendMessage(QHostAddress, quint16, QVariantMap);
  void receiveMessage();
  void addPeer(QString);
  void routeRumor();
  void shareFile(QString);
signals:
  void postMessage(QString, QString, QString);
  void newOrigin(QString);
  void searchReply(QByteArray, QString, QString);
  void receivedBlocklist(QByteArray, qint64);
  void receivedBlock(QByteArray, qint64);
};

#endif
