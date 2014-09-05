#ifndef PEERCONNECTION_HH
#define PEERCONNECTION_HH

#include <QObject>

class PeerConnection : public QObject
{
  Q_OBJECT

private:
  QVariantMap msg;
  bool rumor;
  bool waiting;
public:
  PeerConnection();
  PeerConnection(QVariantMap);
  bool connected();
  void connect();
  void waiting();
};

#endif
