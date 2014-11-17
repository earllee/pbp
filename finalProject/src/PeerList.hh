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
  QMap<QByteArray, bool> *results;
  QTimer *searchTimer;
  Peer *get(QHostAddress, quint16);
  Peer *random();
  QList<Peer*> randoms(quint32);
  void propagateSearch(QVariantMap&, quint32);
  void handleSearchRequest(QVariantMap&, Origin*, quint32);
  void handleSearchReply(QVariantMap&, Origin*, Origin*, quint32);
  void handlePrivate(QVariantMap&, Origin*, Origin*, quint32);
  void handleRumor(QVariantMap&, Peer*, Origin*);
  void handleStatus(QVariantMap&, Peer*);
  void handleBlockRequest(QVariantMap&, Origin*, Origin*, quint32);
  void handleBlockReply(QVariantMap&, Origin*, Origin*, quint32);
  void forwardMessage(QVariantMap&, Origin*, quint32);
  QVariantMap extractMessage(QVariantMap&);
  void insertMessage(QVariantMap&, QVariantMap);
public:
  PeerList(quint16, bool nf = false);
  ~PeerList();
  Peer *add(QHostAddress, QString, quint16);
  Peer *add(QHostAddress, quint16);
  Peer *add(QString, quint16);
  void newMessage(QHostAddress, quint16, QVariantMap&);
  Peer *getMe();
  void setMe(QHostAddress, quint16);
  QHostAddress myHost();
  quint16 myPort();
  QString myName();
  quint32 mySeqNo();
  void shareFile(QString);
  void startDownload(QByteArray, QString, QString);
public slots:
  void rumor(QVariantMap, bool broadcast = false);
  void sentMessage(QHostAddress, quint16);
  void antiEntropy();
  void expandSearch();
signals:
  void sendMessage(QHostAddress, quint16, QVariantMap);
  void postMessage(QString, QString, QString);
  void newOrigin(QString);
  void searchReply(QByteArray, QString, QString);
  void receivedBlocklist(QByteArray, qint64);
  void receivedBlock(QByteArray, qint64);
};

#endif
