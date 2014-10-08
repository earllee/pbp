#include <unistd.h>

#include <Origin.hh>
#include <SharedFile.hh>
#include <QVector>
#include <QVariant>
#include <QFileInfo>
#include <QDebug>

#define HOPLIMIT 10

Origin::Origin(QString n, Peer *h) {
  name = n;
  seqNo = 1;
  latestSeqNo = 0;
  color.setHsv(qrand() % 360, 128 + qrand() % 128, (qrand() % 128) + 64);
  messages = new QVector<QVariantMap>();
  files = new QMap<QByteArray, SharedFile*>();
  downloads = new QMap<QByteArray, SharedFile*>();
  hop = h;
  direct = false;
  qDebug() << QString("New hop for %1 is %2:%3").arg(name).arg(hop->getHost().toString()).arg(hop->getPort());
}

Origin::~Origin() {
  delete messages;
  foreach(SharedFile *file, files->values()) {
    delete file;
  }
  delete files;
  foreach(SharedFile *file, downloads->values()) {
    delete file;
  }
  delete downloads;
}

QString Origin::getName() {
  return name;
}

quint32 Origin::next() {
  return seqNo;
}

QVariantMap Origin::message(quint32 sn) {
  return messages->value(sn - 1);
}

bool Origin::addMessage(quint32 sn, QVariantMap datagram, Peer *sender, bool dir) {
  if((int)sn > messages->size())
    messages->resize(sn);
  if(messages->value(sn - 1).empty() && sn >= seqNo) {
    // set the new message if it doesn't already exist
    messages->replace(sn - 1, datagram);
    if (sn == seqNo) {
      // if the seqNo is the one that is needed then increment need until
      // the first missing message (in case later messages were received
      // first)
      for(; !messages->value(seqNo - 1).empty(); seqNo++) {
	// emit all messages that were waiting if not route rumor
	if(messages->value(seqNo-1).contains("ChatText"))
	  emit postMessage(name, messages->value(seqNo-1).value("ChatText").toString(), color, QString());
      }
    }
    // update if newest otherwise update if direct but wasn't already direct
    if (sn > latestSeqNo || (sn == latestSeqNo && dir && !direct)) {
      latestSeqNo = sn;
      hop = sender;
      qDebug() << QString("New hop for %1 is %2:%3").arg(name).arg(hop->getHost().toString()).arg(hop->getPort());
    }
    return true;
  } else {
    return false;
  }
}

Peer *Origin::getHop() {
  return hop;
}

void Origin::privateMessage(QVariantMap datagram, QString chatbox) {
  emit postMessage(name, datagram.value("ChatText").toString(), color, chatbox);
}

void Origin::shareFile(QString filename) {
  SharedFile *file = new SharedFile(filename);
  files->insert(file->getMeta(), file);
}

SharedFile *Origin::fileByHash(QByteArray metaHash) {
  return files->value(metaHash);
}

void Origin::startDownload(QString filename, QByteArray meta) {
  files->insert(meta, new SharedFile(filename, meta));
}

QList<SharedFile*> Origin::searchFiles(QString query) {
  QList<SharedFile*> results;
  QStringList keywords = query.split(" ", QString::SkipEmptyParts);
  QStringList fileKeywords;
  bool add;
  foreach(SharedFile *file, files->values()) {
    add = false;
    fileKeywords = QFileInfo(file->getFilename()).fileName().split(" ", QString::SkipEmptyParts);
    foreach(QString keyword, keywords) {
      foreach(QString fileKeyword, fileKeywords) {
	if (keyword == fileKeyword) {
	  add = true;
	  break;
	}
      }
      if (add) {
	results.append(file);
	break;
      }
    }
  }
  return results;
}

void Origin::blockRequest(QVariantMap datagram, Origin *me) {
  QByteArray blockHash = datagram.value("BlockRequest").toByteArray();
  SharedFile *file = downloads->value(blockHash);
  if (!file) {
    SharedFile *fileToDl = me->fileByHash(blockHash);
    if (!fileToDl)
      return;
    file = new SharedFile(fileToDl->getFilename(), fileToDl->getMeta(), fileToDl->getBlocklist());
  }
  QByteArray next;
  QByteArray data = file->blockRequest(blockHash, &next);
  downloads->remove(blockHash);
  if (!data.isEmpty()) {
    if (!next.isEmpty())
      downloads->insert(next, file);
    QVariantMap send;
    send.insert("Dest", QVariant(name));
    send.insert("Origin", QVariant(me->getName()));
    send.insert("HopLimit", QVariant(HOPLIMIT));
    send.insert("BlockReply", QVariant(blockHash));
    send.insert("Data", QVariant(data));
    emit sendMessage(hop->getHost(), hop->getPort(), send);
  }
}

void Origin::blockReply(QVariantMap datagram, Origin *me) {
  QByteArray blockHash = datagram.value("BlockReply").toByteArray();
  QByteArray data = datagram.value("Data").toByteArray();
  SharedFile *file = files->value(blockHash);
  if (!file)
    return;
  QByteArray next = file->blockReply(blockHash, data);
  downloads->remove(blockHash);
  if (!next.isEmpty()) {
    files->insert(next, file);
    QVariantMap send;
    send.insert("Dest", QVariant(name));
    send.insert("Origin", QVariant(me->getName()));
    send.insert("HopLimit", QVariant(HOPLIMIT));
    send.insert("BlockRequest", QVariant(next));
    emit sendMessage(hop->getHost(), hop->getPort(), send);
  }
}
