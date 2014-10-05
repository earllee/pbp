#include <unistd.h>

#include <QtCrypto>
#include <SharedFile.hh>
#include <QByteArray>
#include <QFile>
#include <QIODevice>
#include <QDebug>

#define HASHSIZE 20

SharedFile::SharedFile(QString fn) {
  filename = fn;
  QFile file(filename);
  file.open(QIODevice::ReadOnly);
  fileSize = file.size();
  qint64 blockSize = 1<<13;
  qint64 blocklistSize = (fileSize % blockSize) ?
    (fileSize / blockSize + 1) * HASHSIZE :
    fileSize / blockSize * HASHSIZE;
  blocklist = new QByteArray(0, blocklistSize);
  QCA::Hash sha1("sha1");
  QByteArray block;
  for (int i = 0; i < blocklistSize; i += HASHSIZE) {
    sha1.clear();
    sha1.update(file.read(blockSize));
    block = sha1.final().toByteArray();
    for (int j = 0; j < HASHSIZE; j++)
      (*blocklist)[i + j] = block[j];
  }
  sha1.clear();
  sha1.update(*blocklist);
  blocklistHash = new QByteArray(sha1.final().toByteArray());
}

SharedFile::~SharedFile() {
  delete blocklist;
  delete blocklistHash;
}
