#ifndef CHATDIALOG_HH
#define CHATDIALOG_HH

#include <QDialog>
#include <QTextEdit>
#include <QLineEdit>
#include <QColor>
#include <ChatQTextEdit.hh>

class ChatDialog : public QDialog {
  Q_OBJECT

public:
  ChatDialog();
  ~ChatDialog();

public slots:
  void gotReturnPressed();
  void postMessage(QString, QString, QColor);
  void newPeer();
signals:
  void newMessage(QString);
  void addPeer(QString);
private:
  QTextEdit *textview;
  ChatQTextEdit *textline;
  QLineEdit *peerInput;
};

#endif
