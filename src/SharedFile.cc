#include <unistd.h>

#include <QtCrypto>
#include <SharedFile.hh>
#include <QByteArray>
#include <QFile>
#include <QIODevice>
#include <QDir>
#include <QDebug>

#define HASHSIZE 20
#define BLOCKSIZE (1<<13)

SharedFile::SharedFile(QString fn) {
  filename = fn;
  QFile file(fn);
  file.open(QIODevice::ReadOnly);
  fileSize = file.size();
  blocklistSize = (fileSize % BLOCKSIZE) ?
    (fileSize / BLOCKSIZE + 1) * HASHSIZE :
    fileSize / BLOCKSIZE * HASHSIZE;
  blocklist = new QByteArray(0, blocklistSize);
  QCA::Hash sha1("sha1");
  QByteArray block;
  for (int i = 0; i < blocklistSize; i += HASHSIZE) {
    sha1.clear();
    sha1.update(file.read(BLOCKSIZE));
    block = sha1.final().toByteArray();
    for (int j = 0; j < HASHSIZE; j++)
      (*blocklist)[i + j] = block[j];
  }
  sha1.clear();
  sha1.update(*blocklist);
  blocklistHash = new QByteArray(sha1.final().toByteArray());
  currentBlock = 0;
  qDebug() << "New shared file hash" << blocklistHash->toHex();
}

SharedFile::SharedFile(QString f, QString fn, QByteArray meta, QByteArray bl) {
  from = f;
  filename = fn;
  blocklist = new QByteArray(bl);
  blocklistHash = new QByteArray(meta);
  blocklistSize = blocklist->size();
  currentBlock = 0;
}

SharedFile::~SharedFile() {
  delete blocklist;
  delete blocklistHash;
}

QByteArray SharedFile::blockRequest(QByteArray blockHash, QByteArray *next) {
  // requesting blockfile
  if (currentBlock == 0 && blockHash == *blocklistHash) {
    *next = blocklist->left(HASHSIZE);
    return *blocklist;
  }

  if (blockHash != blocklist->mid(currentBlock * HASHSIZE, HASHSIZE))
    return QByteArray();
  if (currentBlock + 1 < blocklistSize / HASHSIZE)
    *next = blocklist->mid((currentBlock + 1) * HASHSIZE, HASHSIZE);
  else
    *next = QByteArray();
  QFile file(filename);
  file.open(QIODevice::ReadOnly);
  file.seek(currentBlock * BLOCKSIZE);
  currentBlock++;
  return file.read(BLOCKSIZE);
}

QByteArray SharedFile::blockReply(QByteArray blockHash, QByteArray data) {
  // receiving blockfile
  if (blocklist->isEmpty() && blockHash == *blocklistHash) {
    blocklist = new QByteArray(data);
    return blocklist->left(HASHSIZE);
  }

  // check if right content
  if (blockHash != blocklist->mid(currentBlock * HASHSIZE, HASHSIZE)
      || QCA::Hash("sha1").hash(data).toByteArray() != blockHash)
    return QByteArray();

  QString loc = QString("%1/%2").arg(QDir::currentPath()).arg(filename);
  QFile file(loc);
  file.open(QIODevice::ReadWrite | QIODevice::Append);
  file.write(data);
  currentBlock++;
  if (currentBlock < blocklistSize / HASHSIZE)
    return blocklist->mid(currentBlock + HASHSIZE, HASHSIZE);
  else
    return QByteArray();
}

QString SharedFile::getFilename() {
  return filename;
}

QByteArray SharedFile::getMeta() {
  return *blocklistHash;
}

QByteArray SharedFile::getBlocklist() {
  return *blocklist;
}
