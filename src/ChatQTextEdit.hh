#ifndef CHATQTEXTEDIT_HH
#define CHATQTEXTEDIT_HH

#include <QTextEdit>

class ChatQTextEdit : public QTextEdit {
  Q_OBJECT
public:
  ChatQTextEdit(QWidget *parent) : QTextEdit(parent) {}
protected:
  bool eventFilter(QObject *obj, QEvent *event);
signals:
  void returnPressed();
};

#endif
