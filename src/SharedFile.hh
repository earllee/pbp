#ifndef SHAREDFILE_HH
#define SHAREDFILE_HH

#include <QObject>
#include <QByteArray>

class SharedFile : public QObject {
  Q_OBJECT

private:
  QString filename;
  qint64 filesize;
  QByteArray *blocklist;
  QByteArray *blocklistHash;
public:
  SharedFile(QString);
  ~SharedFile();
};

#endif
