#include <unistd.h>

#include <ChatQTextEdit.hh>
#include <QKeyEvent>

bool ChatQTextEdit::eventFilter(QObject *obj, QEvent *event) {
  if (event->type() == QEvent::KeyPress) {
    QKeyEvent *keyEvent = static_cast<QKeyEvent*>(event);
    switch(keyEvent->key()) {
    case Qt::Key_Enter:
    case Qt::Key_Return:
      emit returnPressed();
      return true;
    }
  }
  return false;
}
