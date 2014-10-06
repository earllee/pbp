#ifndef ORIGINLIST_HH
#define ORIGINLIST_HH

#include <QObject>
#include <QVariant>
#include <QColor>
#include <Origin.hh>
#include <Peer.hh>

class OriginList : public QObject {
  Q_OBJECT

private:
  Origin *me;
  QMap<QString, Origin*> *origins;
  Origin *get(QString);
public:
  OriginList(Peer*);
  ~OriginList();
  bool needMessage(QVariantMap);
  QVariantMap nextNeededMessage(QVariantMap);
  bool addMessage(QVariantMap, Peer*, bool);
  QVariantMap status();
  quint32 mySeqNo();
  QString myName();
  Origin *add(QString, Peer*);
  void privateMessage(QVariantMap, Peer*);
  Peer *nextHop(QString);
  void shareFile(QString);
signals:
  void postMessage(QString, QString, QColor, QString);
  void newOrigin(QString);
  void sendMessage(QHostAddress, quint16, QVariantMap);
};

#endif
