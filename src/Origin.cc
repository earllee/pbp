#include <unistd.h>

#include <Origin.hh>
#include <QVector>
#include <QVariant>
#include <QDebug>

Origin::Origin(QString n) {
  name = n;
  seqNo = 1;
  messages = new QVector<QString>();
}

Origin::~Origin() {
  delete messages;
}

QString Origin::getName() {
  return name;
}

quint32 Origin::next() {
  return seqNo;
}

QVariantMap Origin::message(quint32 sn) {
  QVariantMap datagram;
  datagram.insert("ChatText", QVariant(messages->value(sn - 1)));
  datagram.insert("Origin", QVariant(name));
  datagram.insert("SeqNo", QVariant(sn));
  return datagram;
}

bool Origin::addMessage(quint32 sn, QString text) {
  if((int)sn > messages->size()) {
    messages->resize(sn);
  }
  if(messages->value(sn - 1) == "" && sn >= seqNo) {
    // set the new message if it doesn't already exist
    messages->replace(sn - 1, text);
    if(sn == seqNo) {
      // if the seqNo is the one that is needed then increment need until
      // the first missing message (in case later messages were received
      // first)
      for(; messages->value(seqNo - 1) != ""; seqNo++) {
	// emit all messages that were waiting
	QString msg = QString("%1: %2").arg(name).arg(messages->value(seqNo-1));
	qDebug() << msg;
	emit chatMessage(msg);
      }
    }
    return true;
  } else {
    return false;
  }
}
