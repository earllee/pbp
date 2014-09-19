#include <unistd.h>

#include <ChatTab.hh>
#include <QColor>
#include <QFont>
#include <QVBoxLayout>
#include <QDebug>

ChatTab::ChatTab(QString n) {
  name = n;

  // Read-only text box where we display messages from everyone.
  // This widget expands both horizontally and vertically.
  textview = new QTextEdit(this);
  textview->setReadOnly(true);

  // Small text-entry box the user can enter messages.
  // This widget normally expands only horizontally,
  // leaving extra vertical space for the textview widget.
  //
  // You might change this into a read/write QTextEdit,
  // so that the user can easily enter multi-line messages.
  textline = new ChatQTextEdit(this);
  textline->setMaximumHeight(3 * textline->font().pointSize() + 32);
  textline->installEventFilter(textline);

  this->setFlat(true);
  layout = new QVBoxLayout(this);
  layout->addWidget(textview);
  layout->addWidget(textline);

  if(name.isEmpty())
    // set focus on text input if broadcast
    textline->setFocus();

  // Register a callback on the textline's returnPressed signal
  // so that we can send the message entered by the user.
  connect(textline, SIGNAL(returnPressed()),
	  this, SLOT(gotReturnPressed()));
}

ChatTab::~ChatTab() {
  delete textline;
  delete textview;
  delete layout;
}

void ChatTab::gotReturnPressed() {
  if(textline->toPlainText() != "") {
    emit newMessage(textline->toPlainText(), name);

    // Clear the textline to get ready for the next input message.
    textline->clear();
  }
}

void ChatTab::postMessage(QString name, QString msg, QColor color) {
  textview->setTextColor(color);
  textview->setFontWeight(QFont::Bold);
  textview->append(QString("[%1] ").arg(name));
  textview->setFontWeight(QFont::Normal);
  textview->insertPlainText(msg);
  textview->moveCursor(QTextCursor::End);
}
