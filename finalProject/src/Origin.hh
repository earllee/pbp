#ifndef ORIGIN_HH
#define ORIGIN_HH

#include <QObject>
#include <QVariant>
#include <QColor>
#include <Peer.hh>
#include <SharedFile.hh>

class Origin : public QObject {
  Q_OBJECT

private:
  QString name;
  quint32 seqNo;
  quint32 latestSeqNo;
  QColor color;
  QVector<QVariantMap> *messages;
  // if the origin is me then files are the files that I'm sharing where the
  // key is the metafile's hash
  // if the origin is not me then files are the files that I'm downloading from
  // that origin where the key is the hash of the next expected block reply
  QMap<QByteArray, SharedFile*> *files;
  // if the origin is me then downloads is not in use
  // if the origin is not me then files are the files that are currently being
  // downloaded from me by that origin where the key is the next expected
  // block request
  QMap<QByteArray, SharedFile*> *downloads;
  Peer *hop;
  bool direct;
public:
  Origin(QString, Peer*);
  ~Origin();
  QString getName();
  quint32 next();
  Peer *getHop();
  QVariantMap message(quint32);
  bool addMessage(quint32, QVariantMap, Peer*, bool);
  QVariantMap blockRequest(QVariantMap, Origin*);
  QVariantMap blockReply(QVariantMap);
  void shareFile(QString, bool);
  void startDownload(QByteArray, QString);
  SharedFile *fileByHash(QByteArray);
  QList<SharedFile*> searchFiles(QString, bool);
signals:
  void postMessage(QString, QString, QString);
  void receivedBlocklist(QByteArray, qint64);
  void receivedBlock(QByteArray, qint64);
};

#endif
