#ifndef ORIGINLIST_HH
#define ORIGINLIST_HH

#include <QObject>
#include <Origin.hh>

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
