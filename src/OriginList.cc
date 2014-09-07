#include <unistd.h>

#include <OriginList.hh>
#include <Origin.hh>
#include <QVector>
#include <QVariant>
#include <QTime>

OriginList::OriginList() {
  qsrand(QTime::currentTime().msec());
  me = new Origin(QString("yoloswag%1").arg(qrand()));
  origins = new QVector<Origin*>();
}

OriginList::~OriginList() {
  foreach(Origin *o, *origins) {
    delete o;
  }
  delete origins;
  delete me;
}

Origin *OriginList::get(QString name) {
  if(name == myName()) {
    return me;
  }
  foreach(Origin *o, *origins) {
    if(o->getName() == name) {
      return o;
    }
  }
 return NULL;
}

Origin *OriginList::add(QString name) {
  Origin *newOrigin = new Origin(name);
  origins->append(newOrigin);
  return newOrigin;
}

bool OriginList::needMessage(QVariantMap want) {
  QVariantMap status = want.value("Want").toMap();
  Origin *o;
  bool need = false;
  foreach(QString name, status.keys()) {
    o = get(name);
    if(o == NULL) {
      add(name);
      need = true;
    } else {
      if(status.value(name).toUInt() - 1 >= o->next()) {
	need = true;
      }
    }
  }
  return need;
}

QVariantMap OriginList::nextNeededMessage(QVariantMap want) {
  QVariantMap status = want.value("Want").toMap();
  Origin *o;
  foreach(QString name, status.keys()) {
    o = get(name);
    if(o != NULL) {
      uint needed = status.value(name).toUInt();
      if(needed < o->next()) {
	return o->message(needed);
      }
    }
  }
  return QVariantMap();
}

bool OriginList::addMessage(QVariantMap message) {
  QString name = message.value("Origin").toString();
  Origin *o = get(name);
  if(o == NULL) {
    o = add(name);
  }
  return o->addMessage(message.value("SeqNo").toUInt(), message.value("ChatText").toString());
}

QVariantMap OriginList::status() {
  QVariantMap status;
  status.insert(myName(), mySeqNo());
  foreach(Origin *o, *origins) {
    status.insert(o->getName(), QVariant(o->next()));
  }
  QVariantMap want;
  want.insert("Want", QVariant(status));
  return want;
}

QString OriginList::myName() {
  return me->getName();
}

quint32 OriginList::mySeqNo() {
  return me->next();
}
