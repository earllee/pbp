#ifndef SHAREDFILE_HH
#define SHAREDFILE_HH

#include <QObject>
#include <QByteArray>

class SharedFile : public QObject {
  Q_OBJECT

private:
  QString filename;
  qint64 fileSize;
  qint64 blocklistSize;
  QByteArray *blocklist;
  QByteArray *blocklistHash;
  qint64 currentBlock;
  QString from;
public:
  SharedFile(QString);
  SharedFile(QString, QByteArray, QByteArray bl = QByteArray());
  ~SharedFile();
  QByteArray blockRequest(QByteArray, QByteArray*);
  QByteArray blockReply(QByteArray, QByteArray);
  QString getFilename();
  QByteArray getMeta();
  QByteArray getBlocklist();
signals:
  void downloadProgress(QByteArray, quint16);
};

#endif
