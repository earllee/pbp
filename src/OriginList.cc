#include <unistd.h>

#include <OriginList.hh>
#include <Origin.hh>
#include <QVariant>
#include <QTime>
#include <QColor>
#include <QVector>
#include <QFileInfo>
#include <QDebug>

#define HOPLIMIT 10

QVector<QString> POKEMON = QVector<QString>() << "Bulbasaur" << "Ivysaur" << "Venusaur" << "Charmander" << "Charmeleon" << "Charizard" << "Squirtle" << "Wartortle" << "Blastoise" << "Caterpie" << "Metapod" << "Butterfree" << "Weedle" << "Kakuna" << "Beedrill" << "Pidgey" << "Pidgeotto" << "Pidgeot" << "Rattata" << "Raticate" << "Spearow" << "Fearow" << "Ekans" << "Arbok" << "Pikachu" << "Raichu" << "Sandshrew" << "Sandslash" << "Nidoran♀" << "Nidorina" << "Nidoqueen" << "Nidoran♂" << "Nidorino" << "Nidoking" << "Clefairy" << "Clefable" << "Vulpix" << "Ninetales" << "Jigglypuff" << "Wigglytuff" << "Zubat" << "Golbat" << "Oddish" << "Gloom" << "Vileplume" << "Paras" << "Parasect" << "Venonat" << "Venomoth" << "Diglett" << "Dugtrio" << "Meowth" << "Persian" << "Psyduck" << "Golduck" << "Mankey" << "Primeape" << "Growlithe" << "Arcanine" << "Poliwag" << "Poliwhirl" << "Poliwrath" << "Abra" << "Kadabra" << "Alakazam" << "Machop" << "Machoke" << "Machamp" << "Bellsprout" << "Weepinbell" << "Victreebel" << "Tentacool" << "Tentacruel" << "Geodude" << "Graveler" << "Golem" << "Ponyta" << "Rapidash" << "Slowpoke" << "Slowbro" << "Magnemite" << "Magneton" << "Farfetch'd" << "Doduo" << "Dodrio" << "Seel" << "Dewgong" << "Grimer" << "Muk" << "Shellder" << "Cloyster" << "Gastly" << "Haunter" << "Gengar" << "Onix" << "Drowzee" << "Hypno" << "Krabby" << "Kingler" << "Voltorb" << "Electrode" << "Exeggcute" << "Exeggutor" << "Cubone" << "Marowak" << "Hitmonlee" << "Hitmonchan" << "Lickitung" << "Koffing" << "Weezing" << "Rhyhorn" << "Rhydon" << "Chansey" << "Tangela" << "Kangaskhan" << "Horsea" << "Seadra" << "Goldeen" << "Seaking" << "Staryu" << "Starmie" << "Mr. Mime" << "Scyther" << "Jynx" << "Electabuzz" << "Magmar" << "Pinsir" << "Tauros" << "Magikarp" << "Gyarados" << "Lapras" << "Ditto" << "Eevee" << "Vaporeon" << "Jolteon" << "Flareon" << "Porygon" << "Omanyte" << "Omastar" << "Kabuto" << "Kabutops" << "Aerodactyl" << "Snorlax" << "Articuno" << "Zapdos" << "Moltres" << "Dratini" << "Dragonair" << "Dragonite" << "Mewtwo" << "Mew";

QVector<QString> PERSONALITIES = QVector<QString>() << "Hardy" << "Lonely" << "Brave" << "Adamant" << "Naughty" << "Bold" << "Docile" << "Relaxed" << "Impish" << "Lax" << "Timid" << "Hasty" << "Serious" << "Jolly" << "Naive" << "Modest" << "Mild" << "Quiet" << "Bashful" << "Rash" << "Calm" << "Gentle" << "Sassy" << "Careful" << "Quirky";

OriginList::OriginList(Peer *myPeer) {
  qsrand(QTime::currentTime().msec());
  me = new Origin(QString("%1%2%3").arg(PERSONALITIES.value(qrand() % PERSONALITIES.size())).arg(POKEMON.value(qrand() % POKEMON.size())).arg(qrand() % 100), myPeer);
  connect(me, SIGNAL(postMessage(QString, QString, QColor, QString)),
	  this, SIGNAL(postMessage(QString, QString, QColor, QString)));
  connect(me, SIGNAL(sendMessage(QHostAddress, quint16, QVariantMap)),
	  this, SIGNAL(sendMessage(QHostAddress, quint16, QVariantMap)));
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

Origin *OriginList::add(QString name, Peer *sender) {
  Origin *o = new Origin(name, sender);
  connect(o, SIGNAL(postMessage(QString, QString, QColor, QString)),
	  this, SIGNAL(postMessage(QString, QString, QColor, QString)));
  connect(o, SIGNAL(sendMessage(QHostAddress, quint16, QVariantMap)),
	  this, SIGNAL(sendMessage(QHostAddress, quint16, QVariantMap)));
  connect(me, SIGNAL(receivedBlocklist(QByteArray, qint64)),
	  this, SIGNAL(receivedBlocklist(QByteArray, qint64)));
  connect(me, SIGNAL(receivedBlock(QByteArray, qint64)),
	  this, SIGNAL(receivedBlock(QByteArray, qint64)));
  origins->insert(name, o);
  emit newOrigin(name);
  return o;
}

bool OriginList::needMessage(QVariantMap want) {
  QVariantMap status = want.value("Want").toMap();
  Origin *o;
  bool need = false;
  foreach(QString name, status.keys()) {
    o = get(name);
    if(!o) {
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
  QMap<QString, Origin*> allOrigins(*origins);
  allOrigins.insert(myName(), me);
  QVariant v;
  quint32 needed;
  foreach(Origin *o, allOrigins) {
    v = status.value(o->getName());
    if (v.isValid())
      needed = v.toUInt();
    else
      needed = 1;
    if(needed < o->next()) {
      return o->message(needed);
    }
  }
  return QVariantMap();
}

bool OriginList::addMessage(QVariantMap message, Peer *sender, bool direct) {
  QString name = message.value("Origin").toString();
  Origin *o = get(name);
  if(!o)
    o = add(name, sender);
  return o->addMessage(message.value("SeqNo").toUInt(), message, sender, direct);
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

void OriginList::startDownload(QString filename, QByteArray meta, QString dest) {
  Origin *o = get(dest);
  if (!o)
    return;
  o->startDownload(filename, meta);
}

void OriginList::privateMessage(QVariantMap datagram, Peer *sender) {
  QString from = datagram.value("Origin").toString();
  Origin *o;
  if (datagram.contains("BlockRequest")) {
    o = get(from);
    if (!o)
      o = add(from, sender);
    o->blockRequest(datagram, me);
  } else if (datagram.contains("BlockReply")) {
    o = get(from);
    if (!o)
      o = add(from, sender);
    o->blockReply(datagram, me);
  } else if (datagram.contains("SearchReply")) {
    o = get(from);
    if (!o)
      o = add(from, sender);
    QVariantList filenames = datagram.value("MatchNames").toList();
    QVariantList ids = datagram.value("MatchIDs").toList();
    int len = filenames.size();
    for (int i = 0; i < len; i++) {
      QVariantMap reply;
      reply.insert("Filename", filenames.at(i));
      reply.insert("ID", ids.at(i));
      reply.insert("Origin", QVariant(from));
      reply.insert("SearchReply", datagram.value("SearchReply"));
      emit searchReply(reply);
    }
  } else {
    QString chatbox;
    if (from == myName()) {
      o = me;
      chatbox = datagram.value("Dest").toString();
    } else {
      o = get(from);
      if (!o)
	o = add(from, sender);
      chatbox = from;
    }
    o->privateMessage(datagram, chatbox);
  }
}

void OriginList::searchMessage(QVariantMap datagram, Peer *sender) {
  QString from = datagram.value("Origin").toString();
  Origin *o = get(from);
  if (!o)
    o = add(from, sender);
  QString query = datagram.value("Search").toString();
  QList<SharedFile*> files = me->searchFiles(query);
  QVariantList filenames, ids;
  foreach(SharedFile *file, files) {
    filenames.append(QVariant(QFileInfo(file->getFilename()).fileName()));
    ids.append(QVariant(file->getMeta()));
  }
  QVariantMap reply;
  reply.insert("Dest", QVariant(from));
  reply.insert("Origin", QVariant(me->getName()));
  reply.insert("HopLimit", QVariant(HOPLIMIT));
  reply.insert("SearchReply", QVariant(query));
  reply.insert("MatchNames", QVariant(filenames));
  reply.insert("MatchIDs", QVariant(ids));
  send(from, reply);
}

void OriginList::send(QString dest, QVariantMap datagram) {
  Origin *o = get(dest);
  if (!o)
    return;
  Peer *hop = o->getHop();
  emit sendMessage(hop->getHost(), hop->getPort(), datagram);
}

void OriginList::shareFile(QString filename) {
  me->shareFile(filename);
}
