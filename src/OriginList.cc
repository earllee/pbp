#include <unistd.h>

#include <OriginList.hh>
#include <QVector>

OriginList::OriginList() {
  // fix later
  me = new Origin("lololol");
  origins = new QVector<*Origin>();
}

OriginList::~OriginList() {
  foreach(Origin *o, origins) {
    delete o;
  }
  delete origins;
  delete me;
}

Origin *OriginList::get(QString name) {
 foreach(Origin *o, origins) {
   if(o->name() == name) {
     return o;
   }
 }
 return nullptr;
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
    if(o == nullptr) {
      add(name);
      need = true;
    } else {
      if(status.value(name).toUInt() >= o->next()) {
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
    if(o != nullptr) {
      int needed = status.value(name).toUInt();
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
  if(o == nullptr) {
    o = add(name);
  }
  return o->addMessage(message.value("SeqNo").toUInt(), message.value("ChatText").toString());
}

QVariantMap OriginList::status() {
  QVariantMap status;
  foreach(Origin *o, origins) {
    status.insert(o->name(), QVariant(origin->next()));
  }
  QVariantMap want;
  want.insert("Want", QVariant(status));
  return want;
}
