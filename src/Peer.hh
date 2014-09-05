#ifndef PEER_HH
#define PEER_HH

#include <QObject>
#include <QHostAddress>
#include <QVarLengthArray>

class Peer : public QObject
{
  Q_OBJECT

private:
  QString name;
  QHostAddress host;
  quint16 port;
  QVarLengthArray<QString> messages;
  quint32 need;
public:
  Peer(QString, QHostAddress, quint16);
  Add(quint32 seqNo, QString text);
  Get(quint32 seqNo);
  Next();
};

#endif
