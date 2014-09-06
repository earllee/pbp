#ifndef PEERLIST_HH
#define PEERLIST_HH

#include <QObject>
#include <QHostAddress>
#include <Peer.hh>

class OriginList : public QObject
{
  Q_OBJECT

private:
  Origin *me;
  QVector<*Origin> *origins;
  Origin *get(QString);
public:
  OriginList();
  ~OriginList();
  bool needMessage(QVariantMap);
  QVariantMap nextNeededMessage(QVariantMap);
  bool addMessage(QVariantMap);
  QVariantMap status();
};

#endif
