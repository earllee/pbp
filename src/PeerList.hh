#ifndef PEERLIST_HH
#define PEERLIST_HH

#include <QObject>
#include <QHostAddress>
#include <QTimer>
#include <QColor>
#include <Peer.hh>
#include <OriginList.hh>

class PeerList : public QObject {
  Q_OBJECT

private:
  Peer *me;
  bool nofwd;
  QMap<QString, Peer*> *peers;
  OriginList *origins;
  QTimer *entropyTimer;
  QVariantMap currentQuery;
  quint32 currentBudget;
  quint32 nResults;
  QTimer *searchTimer;
  Peer *get(QHostAddress, quint16);
  Peer *random();
  QList<Peer*> randoms();
  void propagateSearch(QVariantMap, quint32);
  void handleSearch(QVariantMap, Peer*);
  void handlePrivate(QVariantMap, Peer*);
  void handleStatus(QVariantMap, Peer*);
  void handleRumor(QVariantMap, Peer*);
public:
  PeerList(quint16, bool nf = false);
  ~PeerList();
  Peer *add(QHostAddress, QString, quint16);
  Peer *add(QHostAddress, quint16);
  Peer *add(QString, quint16);
  void newMessage(QHostAddress, quint16, QVariantMap);
  Peer *getMe();
  void setMe(QHostAddress, quint16);
  QHostAddress myHost();
  quint16 myPort();
  QString myName();
  quint32 mySeqNo();
  void shareFile(QString);
  void startDownload(QString, QByteArray, QString);
public slots:
  void rumor(QVariantMap, bool broadcast = false);
  void sentMessage(QHostAddress, quint16);
  void antiEntropy();
  void gotSearchReply(QVariantMap);
  void expandSearch();
signals:
  void sendMessage(QHostAddress, quint16, QVariantMap);
  void postMessage(QString, QString, QColor, QString);
  void newOrigin(QString);
  void searchReply(QVariantMap);
};

#endif
