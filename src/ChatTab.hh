#ifndef CHATTAB_HH
#define CHATTAB_HH

#include <QGroupBox>
#include <QVBoxLayout>
#include <QTextEdit>
#include <QColor>
#include <ChatQTextEdit.hh>

class ChatTab : public QGroupBox {
  Q_OBJECT

public:
  ChatTab(QString);
  ~ChatTab();

public slots:
  void gotReturnPressed();
  void postMessage(QString, QString, QColor);
signals:
  void newMessage(QString, QString);
private:
  QTextEdit *textview;
  ChatQTextEdit *textline;
  QString name;
  QVBoxLayout *layout;
};

#endif
