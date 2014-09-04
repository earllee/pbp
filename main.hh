#ifndef PEERSTER_MAIN_HH
#define PEERSTER_MAIN_HH

#include <QDialog>
#include <QTextEdit>
#include <QLineEdit>
#include <QUdpSocket>

class ChatQTextEdit : public QTextEdit {
  Q_OBJECT
public:
  ChatQTextEdit(QWidget *parent) : QTextEdit(parent) {}
protected:
  bool eventFilter(QObject *obj, QEvent *event);
signals:
  void returnPressed();
};

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

class NetSocket : public QUdpSocket
{
  Q_OBJECT

public:
  NetSocket();

  // Bind this socket to a Peerster-specific default port.
  bool bind();

private:
  int myPortMin, myPortMax;
  QUdpSocket *udp;

public slots:
  void sendMessage(QString text);
  void receiveMessage();

signals:
  void receivedMessage(QString text);
};

#endif // PEERSTER_MAIN_HH
