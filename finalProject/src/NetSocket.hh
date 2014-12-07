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
  void requestTrust(QString); // send a trust request, of form host:port; 
                              // slot from dialog; host:port
  void trustApproved(QString); // slot from dialog; host:port
                               // reply to approveTrust()
signals:
  void postMessage(QString, QString, QString);
  void newOrigin(QString);
  void newPeer(QString);
  void searchReply(QByteArray, QString, QString);
  void receivedBlocklist(QByteArray, qint64);
  void receivedBlock(QByteArray, qint64);

  void approveTrust(QString); // ask to approve in chat; signal to dialog
                              // Just received a req from else; host:port
 
  void acceptedTrust(QString); // requestee accepts; signal to dialog; host:port
                               // Accepts our own req; reply to your
                               // local req; host:port; second to reqTrust()
  void messageable(QString); // received key for that peer; signal to dialog
                             // origin
};

#endif
