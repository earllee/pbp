#ifndef CHATDIALOG_HH
#define CHATDIALOG_HH

#include <QDialog>
#include <QTextEdit>
#include <QListWidget>
#include <QLineEdit>
#include <QColor>
#include <ChatTab.hh>
#include <ChatQTextEdit.hh>
#include <DownloadBox.hh>
#include <QPushButton>
#include <QFileDialog>
#include <QInputDialog>

class ChatDialog : public QDialog {
  Q_OBJECT

public:
  ChatDialog(bool nofwd = false);
  ChatTab *newChatTab(QString);
public slots:
  void postMessage(QString, QString, QString);
  void addPeer();
  void newPeer(QString);
  void newOrigin(QString);
  void originClicked(QListWidgetItem*);
  void openFileDialog();
  void searchReply(QByteArray, QString, QString);
  void initiateSearch();
  void startDownload(QListWidgetItem*);
  void receivedBlocklist(QByteArray, qint64);
  void receivedBlock(QByteArray, qint64);
  void peerClicked(QListWidgetItem*);
  void approveTrust(QString); // expects host:port
  void acceptedTrust(QString); // expects host:port
  void messageable(QString); // expects origin name
signals:
  void newMessage(QString, QString);
  void addPeer(QString);
  void shareFile(QString);
  void downloadFile(QByteArray, QString, QString);
  void search(QString);
  void requestTrust(QString); // sends host:port
  void trustApproved(QString); // sends host:port
private:
  ChatTab *broadcast;
  QMap<QString, ChatTab*> *chats;
  QMap<QString, QColor> *colors;
  QLineEdit *peerInput;
  QListWidget *originSelect;
  QListWidget *peerSelect;
  QTabWidget *tabs;
  QGroupBox *sharingBox;
  QPushButton *sharingButton;
  QPushButton *sharingSearch;
  QLineEdit *sharingInput;
  QListWidget *sharingResults;
  QListWidget *sharingFiles;
  QGridLayout *sharingLayout;
  QGridLayout *layout;
  QMap<QByteArray, QPair<QString, QString> > *results;
  QMap<QByteArray, DownloadBox*> *downloads;
  void setPeerState(QString, QString);
  void setOriginState(QString, QString);
};

#endif
