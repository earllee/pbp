#ifndef ORIGIN_HH
#define ORIGIN_HH

#include <QObject>
#include <QVariant>
#include <QColor>
#include <Peer.hh>

class Origin : public QObject {
  Q_OBJECT

private:
  QString name;
  quint32 seqNo;
  quint32 latestSeqNo;
  QColor color;
  QVector<QVariantMap> *messages;
  Peer *hop;
public:
  Origin(QString, Peer*);
  ~Origin();
  QString getName();
  quint32 next();
  Peer *getHop();
  QVariantMap message(quint32);
  bool addMessage(quint32, QVariantMap, Peer*);
signals:
  void postMessage(QString, QString, QColor);
};

#endif
