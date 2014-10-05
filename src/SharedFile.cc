#include <unistd.h>

#include <QtCrypto>
#include <SharedFile.hh>
#include <QByteArray>
#include <QFile>
#include <QIODevice>
#include <QDebug>

SharedFile::SharedFile(QString fn) {
  filename = fn;
  QFile file(filename);
  file.open(QIODevice::ReadOnly);
  filesize = file.size();
  qint64 blocksize = 1<<13;
  blocklist = new QByteArray();
  QCA::Hash sha1("sha1");
  while (!file.atEnd()) {
    sha1.clear();
    sha1.update(file.read(blocksize));
    blocklist->append(sha1.final().toByteArray());
  }
  sha1.clear();
  sha1.update(*blocklist);
  blocklistHash = new QByteArray(sha1.final().toByteArray());
}

SharedFile::~SharedFile() {
  delete blocklist;
  delete blocklistHash;
}
