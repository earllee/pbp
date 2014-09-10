#include <unistd.h>

#include <Origin.hh>
#include <QVector>
#include <QVariant>
#include <QDebug>

Origin::Origin(QString n) {
  name = n;
  seqNo = 1;
  messages = new QVector<QVariantMap>();
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
  return messages->value(sn - 1);
}

bool Origin::addMessage(quint32 sn, QVariantMap datagram) {
  if((int)sn > messages->size()) {
    messages->resize(sn);
  }
  if(messages->value(sn - 1).empty() && sn >= seqNo) {
    // set the new message if it doesn't already exist
    messages->replace(sn - 1, datagram);
    if(sn == seqNo) {
      // if the seqNo is the one that is needed then increment need until
      // the first missing message (in case later messages were received
      // first)
      for(; !messages->value(seqNo - 1).empty(); seqNo++) {
	// emit all messages that were waiting
	QString msg = QString("%1: %2").arg(name).arg(messages->value(seqNo-1).value("ChatText").toString());
	emit postMessage(msg);
      }
    }
    return true;
  } else {
    return false;
  }
}
