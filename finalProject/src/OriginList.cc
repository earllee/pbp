#include <unistd.h>

#include <OriginList.hh>
#include <Origin.hh>
#include <QVariant>
#include <QTime>
#include <QColor>
#include <QVector>
#include <QFileInfo>
#include <QDebug>

#define HOPLIMIT 10U

QVector<QString> POKEMON = QVector<QString>() << "Bulbasaur" << "Ivysaur" << "Venusaur" << "Charmander" << "Charmeleon" << "Charizard" << "Squirtle" << "Wartortle" << "Blastoise" << "Caterpie" << "Metapod" << "Butterfree" << "Weedle" << "Kakuna" << "Beedrill" << "Pidgey" << "Pidgeotto" << "Pidgeot" << "Rattata" << "Raticate" << "Spearow" << "Fearow" << "Ekans" << "Arbok" << "Pikachu" << "Raichu" << "Sandshrew" << "Sandslash" << "Nidoran♀" << "Nidorina" << "Nidoqueen" << "Nidoran♂" << "Nidorino" << "Nidoking" << "Clefairy" << "Clefable" << "Vulpix" << "Ninetales" << "Jigglypuff" << "Wigglytuff" << "Zubat" << "Golbat" << "Oddish" << "Gloom" << "Vileplume" << "Paras" << "Parasect" << "Venonat" << "Venomoth" << "Diglett" << "Dugtrio" << "Meowth" << "Persian" << "Psyduck" << "Golduck" << "Mankey" << "Primeape" << "Growlithe" << "Arcanine" << "Poliwag" << "Poliwhirl" << "Poliwrath" << "Abra" << "Kadabra" << "Alakazam" << "Machop" << "Machoke" << "Machamp" << "Bellsprout" << "Weepinbell" << "Victreebel" << "Tentacool" << "Tentacruel" << "Geodude" << "Graveler" << "Golem" << "Ponyta" << "Rapidash" << "Slowpoke" << "Slowbro" << "Magnemite" << "Magneton" << "Farfetch'd" << "Doduo" << "Dodrio" << "Seel" << "Dewgong" << "Grimer" << "Muk" << "Shellder" << "Cloyster" << "Gastly" << "Haunter" << "Gengar" << "Onix" << "Drowzee" << "Hypno" << "Krabby" << "Kingler" << "Voltorb" << "Electrode" << "Exeggcute" << "Exeggutor" << "Cubone" << "Marowak" << "Hitmonlee" << "Hitmonchan" << "Lickitung" << "Koffing" << "Weezing" << "Rhyhorn" << "Rhydon" << "Chansey" << "Tangela" << "Kangaskhan" << "Horsea" << "Seadra" << "Goldeen" << "Seaking" << "Staryu" << "Starmie" << "Mr. Mime" << "Scyther" << "Jynx" << "Electabuzz" << "Magmar" << "Pinsir" << "Tauros" << "Magikarp" << "Gyarados" << "Lapras" << "Ditto" << "Eevee" << "Vaporeon" << "Jolteon" << "Flareon" << "Porygon" << "Omanyte" << "Omastar" << "Kabuto" << "Kabutops" << "Aerodactyl" << "Snorlax" << "Articuno" << "Zapdos" << "Moltres" << "Dratini" << "Dragonair" << "Dragonite" << "Mewtwo" << "Mew";

QVector<QString> PERSONALITIES = QVector<QString>() << "Hardy" << "Lonely" << "Brave" << "Adamant" << "Naughty" << "Bold" << "Docile" << "Relaxed" << "Impish" << "Lax" << "Timid" << "Hasty" << "Serious" << "Jolly" << "Naive" << "Modest" << "Mild" << "Quiet" << "Bashful" << "Rash" << "Calm" << "Gentle" << "Sassy" << "Careful" << "Quirky";

OriginList::OriginList(Peer *myPeer) {
  qsrand(QTime::currentTime().msec());
  me = new Origin(QString("%1%2%3").arg(PERSONALITIES.value(qrand() % PERSONALITIES.size())).arg(POKEMON.value(qrand() % POKEMON.size())).arg(qrand() % 100), myPeer);
  connect(me, SIGNAL(postMessage(QString, QString, QString)),
	  this, SIGNAL(postMessage(QString, QString, QString)));
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

Origin *OriginList::get(QString name, Peer *sender) {
  if(name == myName()) {
    return me;
  }
  Origin *result = origins->value(name);
  if (!result)
    result = add(name, sender);
  return result;
}

Origin *OriginList::add(QString name, Peer *sender) {
  Origin *o = new Origin(name, sender);
  connect(o, SIGNAL(postMessage(QString, QString, QString)),
	  this, SIGNAL(postMessage(QString, QString, QString)));
  connect(o, SIGNAL(receivedBlocklist(QByteArray, qint64)),
	  this, SIGNAL(receivedBlocklist(QByteArray, qint64)));
  connect(o, SIGNAL(receivedBlock(QByteArray, qint64)),
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
      QVariantMap msg = o->message(needed);
      msg.insert("Origin", o->getName());
      return msg;
    }
  }
  return QVariantMap();
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

void OriginList::shareFile(QString filename) {
  me->shareFile(filename);
}

QList<SharedFile*> OriginList::searchFiles(QString query) {
  return me->searchFiles(query);
}
