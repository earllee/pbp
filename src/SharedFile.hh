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
  int indexOf(QByteArray);
public:
  SharedFile(QString);
  SharedFile(QByteArray, QByteArray);
  ~SharedFile();
  QByteArray blockRequest(QByteArray, QByteArray*);
  QByteArray blockReply(QByteArray, QByteArray);
  QByteArray meta();
};

#endif
