#include <unistd.h>

#include <ChatDialog.hh>
#include <QVBoxLayout>

ChatDialog::ChatDialog() {
  setWindowTitle("Peerster");

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

  peerInput = new QLineEdit(this);
	

  // Lay out the widgets to appear in the main window.
  // For Qt widget and layout concepts see:
  // http://doc.qt.nokia.com/4.7-snapshot/widgets-and-layouts.html
  QVBoxLayout *layout = new QVBoxLayout();
  layout->addWidget(textview);
  layout->addWidget(textline);
  layout->addWidget(peerInput);
  setLayout(layout);

  // set focus on text input
  textline->setFocus();


  // Register a callback on the textline's returnPressed signal
  // so that we can send the message entered by the user.
  connect(textline, SIGNAL(returnPressed()),
	  this, SLOT(gotReturnPressed()));

  connect(peerInput, SIGNAL(returnPressed()),
	  this, SLOT(newPeer()));
}

ChatDialog::~ChatDialog() {
  delete textline;
  delete textview;
}

void ChatDialog::gotReturnPressed() {
  if(textline->toPlainText() != "") {
    // Initially, just echo the string locally.
    // Insert some networking code here...
    emit newMessage(textline->toPlainText());

    // Clear the textline to get ready for the next input message.
    textline->clear();
  }
}

void ChatDialog::postMessage(QString text) {
  textview->append(text);
}

void ChatDialog::newPeer() {
  QString peer = peerInput->text();
  peerInput->clear();
  emit addPeer(peer);
}
