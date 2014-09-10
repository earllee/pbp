#ifndef ORIGINLIST_HH
#define ORIGINLIST_HH

#include <QObject>
#include <QVariant>
#include <Origin.hh>

class OriginList : public QObject {
  Q_OBJECT

private:
  Origin *me;
  QMap<QString, Origin*> *origins;
  Origin *get(QString);
public:
  OriginList();
  ~OriginList();
  bool needMessage(QVariantMap);
  QVariantMap nextNeededMessage(QVariantMap);
  bool addMessage(QVariantMap);
  QVariantMap status();
  quint32 mySeqNo();
  QString myName();
  Origin *add(QString);
public slots:
  void relayMessage(QString);
signals:
  void postMessage(QString);
};

#endif
