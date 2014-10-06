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
  QFile file(filename);
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
  qDebug() << "New shared file hash " << blocklistHash->toHex();
}

SharedFile::SharedFile(QByteArray meta, QByteArray bl) {
  filename = QString("derp%1").arg(qrand() % 1000);
  blocklist = new QByteArray(bl);
  blocklistHash = new QByteArray(meta);
  blocklistSize = blocklist->size();
}

SharedFile::~SharedFile() {
  delete blocklist;
  delete blocklistHash;
}

int SharedFile::indexOf(QByteArray blockHash) {
  int blockIndex = -1;
  for (int i = 0; i < blocklistSize; i += HASHSIZE) {
    if (blocklist->mid(i, HASHSIZE) == blockHash)
      blockIndex = i;
  }
  return blockIndex;
}

QByteArray SharedFile::blockRequest(QByteArray blockHash, QByteArray *next) {
  // requesting blockfile
  if (blockHash == *blocklistHash) {
    *next = blocklist->left(HASHSIZE);
    return *blocklist;
  }

  int blockIndex = indexOf(blockHash);
  if (blockIndex < 0)
    return QByteArray();
  if (blockIndex + HASHSIZE < blocklistSize)
    *next = blocklist->mid(blockIndex + HASHSIZE, HASHSIZE);
  else
    *next = QByteArray();
  QFile file(filename);
  file.open(QIODevice::ReadOnly);
  file.seek(blockIndex / HASHSIZE * BLOCKSIZE);
  return file.read(BLOCKSIZE);
}

QByteArray SharedFile::blockReply(QByteArray blockHash, QByteArray data) {
  // receiving blockfile
  if (blockHash == *blocklistHash)
    return blocklist->left(HASHSIZE);

  int blockIndex = indexOf(blockHash);
  if (blockIndex < 0)
    return QByteArray();

  // check if right content
  if (QCA::Hash("sha1").hash(data).toByteArray() != blockHash)
    return QByteArray();

  QString loc = QString("%1/%2").arg(QDir::currentPath()).arg(filename);
  QFile file(loc);
  file.open(QIODevice::ReadWrite | QIODevice::Append);
  file.write(data);
  if (blockIndex + HASHSIZE < blocklistSize)
    return blocklist->mid(blockIndex + HASHSIZE, HASHSIZE);
  else
    return QByteArray();
}

QByteArray SharedFile::meta() {
  return *blocklistHash;
}
