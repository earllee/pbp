#ifndef ORIGIN_HH
#define ORIGIN_HH

#include <QObject>
#include <QVariant>

class Origin : public QObject {
  Q_OBJECT

private:
  QString name;
  quint32 seqNo;
  QVector<QVariantMap> *messages;
public:
  Origin(QString);
  ~Origin();
  QString getName();
  quint32 next();
  QVariantMap message(quint32);
  bool addMessage(quint32, QVariantMap);
signals:
  void postMessage(QString);
};

#endif
