#ifndef CHATDIALOG_HH
#define CHATDIALOG_HH

#include <QDialog>
#include <QTextEdit>
#include <ChatQTextEdit.hh>

class ChatDialog : public QDialog
{
  Q_OBJECT

public:
  ChatDialog();

public slots:
  void gotReturnPressed();
  void postMessage(QString text);

signals:
  void newMessage(QString text);

private:
  QTextEdit *textview;
  ChatQTextEdit *textline;
};

#endif
