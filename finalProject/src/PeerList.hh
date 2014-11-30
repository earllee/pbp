#ifndef PEERLIST_HH
#define PEERLIST_HH

#include <QObject>
#include <QHostAddress>
#include <QTimer>
#include <QColor>
#include <Peer.hh>
#include <Origin.hh>
#include <QtCrypto>

class PeerList : public QObject {
  Q_OBJECT

private:
  Peer *me;
  Origin *myOrigin;
  QTimer *entropyTimer;
  bool nofwd;
  
  /* Structures to hold the peers and origins, updated periodically */
  QMap<QString, Peer*> *peers;
  QMap<QString, Origin*> *origins;
  /* Structure here to handle trusted origins, keys, etc */
  
  /* Search Structures */
  QVariantMap currentQuery;
  quint32 currentBudget;           // Used for searches
  QMap<QByteArray, bool> *results; // Used for search req/replies
  QTimer *searchTimer;

  // Origin => Public Key
  QMap<QString, QByteArray> msgableOrigins; // Keys in DER encoding format
  // QMap<QString, QMap<QString, QCA::PublicKey> > pendingTrustMsgs;
  QMap<QString, QVariantMap> pendingTrustMsgs;
  QMap<QString, int> trustedPeers;
 
  /* getOrigin fetches from local origins structure
   * QString refers to user generated name of origin 
   * Peer* refers to the peer that the path to the origin passes thru
   *    or the next hop
   * */ 
  Origin *getOrigin(QString, Peer*); 
  Origin *getOrigin(QString);
  Origin *addOrigin(QString, Peer*);
  
  /* Various methods to fetch Peer(s) */
  Peer *get(QHostAddress, quint16); // Get specified Peer
  Peer *random(); // Random single Peer
  QList<Peer*> randoms(quint32); // Random set of Peers of size of arg
  
  void propagateSearch(QVariantMap&, quint32);

  /* Handles all messages received from remote hosts */
  void handleSearchRequest(QVariantMap&, Origin*, quint32);
  void handleSearchReply(QVariantMap&, Origin*, Origin*, quint32);
  void handlePrivate(QVariantMap&, Origin*, Origin*, quint32);
  void handleRumor(QVariantMap&, Peer*, Origin*);
  void handleStatus(QVariantMap&, Peer*);
  void handleBlockRequest(QVariantMap&, Origin*, Origin*, quint32);
  void handleBlockReply(QVariantMap&, Origin*, Origin*, quint32);
  void handleTrust(QVariantMap&, Peer*);

  void forwardMessage(QVariantMap&, Origin*, quint32);
  QVariantMap constructStatus();
  QVariantMap constructTrustMsg();

  // To be called when adding keys
  void processNewKeys(QVariantMap, Peer *);
public:
  PeerList(quint16, bool nf = false);
  Peer *getMe();
  void setMe(QHostAddress, quint16);
  QHostAddress myHost();
  quint16 myPort();
  QString myName();
  quint32 mySeqNo();
  
  /* Methods to add a peer to local peers data structure */
  Peer *add(QHostAddress, QString, quint16);
  Peer *add(QHostAddress, quint16);
  Peer *add(QString, quint16);
  
  void newMessage(QHostAddress, quint16, QVariantMap&); // recv'd new msg
  void shareFile(QString);
  void startDownload(QByteArray, QString, QString);

  // Send trust request to given peerstring
  void requestTrust(QString);

  void insertMessage(QVariantMap&, QVariantMap); /* Insert second arg into first */
  // void insertMessage(QVariantMap&, QMap<QString, QCA::PublicKey>); /* Insert second arg into first */
  QVariantMap extractMessage(QVariantMap&); 
  // QMap<QString, QCA::PublicKey> extractTrustMessage(QVariantMap&); 

  // To be called after trust approved
  void processPendingsKeys(QString);
public slots:
  void rumor(QVariantMap, bool broadcast = false);
  void sentMessage(QHostAddress, quint16); // Merely sets the wait timer
  void antiEntropy();
  void expandSearch();
signals:
  void sendMessage(QHostAddress, quint16, QVariantMap); // self explanatory
  void postMessage(QString, QString, QString); // Connected to UI I think
                                               // msgText is second arg
  void newOrigin(QString); // Signal to NetSocket to UI; QString is orig name
  void searchReply(QByteArray, QString, QString);
  void receivedBlocklist(QByteArray, qint64);
  void receivedBlock(QByteArray, qint64);
  void newPeer(QString); // Signal to NetSocket to UI; QString is host:port
  // Using method QString("%1:%2").arg(host.toString()).arg(port);
  void messagable(QString);
  void acceptedTrust(QString);
  void approveTrust(QString);
};

#endif
