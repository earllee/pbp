#include <unistd.h>

#include <OriginList.hh>
#include <Origin.hh>
#include <QVariant>
#include <QTime>
#include <QColor>

OriginList::OriginList() {
  qsrand(QTime::currentTime().msec());
  me = new Origin(QString("yoloswag%1").arg(qrand()));
  connect(me, SIGNAL(postMessage(QString, QString, QColor)),
	  this, SLOT(relayMessage(QString, QString, QColor)));
  origins = new QMap<QString, Origin*>();
}

OriginList::~OriginList() {
  foreach(Origin *o, origins->values()) {
    delete o;
  }
  delete origins;
  delete me;
}

Origin *OriginList::get(QString name) {
  if(name == myName()) {
    return me;
  }
  return origins->value(name);
}

Origin *OriginList::add(QString name) {
  Origin *newOrigin = new Origin(name);
  connect(newOrigin, SIGNAL(postMessage(QString, QString, QColor)),
	  this, SLOT(relayMessage(QString, QString, QColor)));
  origins->insert(name, newOrigin);
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
  return o->addMessage(message.value("SeqNo").toUInt(), message);
}

QVariantMap OriginList::status() {
  QVariantMap status;
  status.insert(myName(), mySeqNo());
  foreach(Origin *o, origins->values()) {
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

void OriginList::relayMessage(QString name, QString msg, QColor color) {
  emit postMessage(name, msg, color);
}
