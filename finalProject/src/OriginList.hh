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
public:
  OriginList(Peer*);
  ~OriginList();
  Origin *get(QString);
  Origin *get(QString, Peer*);
  bool needMessage(QVariantMap);
  QVariantMap nextNeededMessage(QVariantMap);
  QVariantMap status();
  quint32 mySeqNo();
  QString myName();
  Origin *add(QString, Peer*);
  void shareFile(QString);
  QList<SharedFile*> searchFiles(QString);
signals:
  void postMessage(QString, QString, QString);
  void newOrigin(QString);
  void receivedBlocklist(QByteArray, qint64);
  void receivedBlock(QByteArray, qint64);
};

#endif
