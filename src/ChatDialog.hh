#ifndef CHATDIALOG_HH
#define CHATDIALOG_HH

#include <QDialog>
#include <QTextEdit>
#include <QListWidget>
#include <QLineEdit>
#include <QColor>
#include <ChatTab.hh>
#include <ChatQTextEdit.hh>
#include <QPushButton>
#include <QFileDialog>
#include <QInputDialog>

class ChatDialog : public QDialog {
  Q_OBJECT

public:
  ChatDialog(bool nofwd = false);
  ~ChatDialog();
  ChatTab *newChatTab(QString);
public slots:
  void postMessage(QString, QString, QColor, QString);
  void newPeer();
  void newOrigin(QString);
  void openTab(QListWidgetItem*);
  void openFileDialog();
  void openDownloadDialog();
signals:
  void newMessage(QString, QString);
  void addPeer(QString);
  void shareFile(QString);
  void downloadFile(QByteArray, QString);
private:
  ChatTab *broadcast;
  QMap<QString, ChatTab*> *chats;
  QLineEdit *peerInput;
  QListWidget *originSelect;
  QTabWidget *tabs;
  QPushButton *fileButton;
  QPushButton *downloadButton;
};

#endif
