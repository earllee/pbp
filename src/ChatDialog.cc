#include <unistd.h>

#include <ChatDialog.hh>
#include <ChatTab.hh>
#include <QColor>
#include <QFont>
#include <QGroupBox>
#include <QGridLayout>
#include <QLabel>
#include <QDebug>

ChatDialog::ChatDialog(bool nofwd = false) {
  setWindowTitle("Peerster");

  chats = new QMap<QString, ChatTab*>();

  peerInput = new QLineEdit(this);
  peerInput->setPlaceholderText("Add a peer");

  originSelect = new QListWidget(this);
  connect(originSelect, SIGNAL(itemDoubleClicked(QListWidgetItem*)),
	  this, SLOT(openTab(QListWidgetItem*)));

  tabs = new QTabWidget();
  broadcast = new ChatTab("");
  tabs->addTab(broadcast, "Broadcast");
  connect(broadcast, SIGNAL(newMessage(QString, QString)),
	  this, SIGNAL(newMessage(QString, QString)));
  
  // Lay out the widgets to appear in the main window.
  // For Qt widget and layout concepts see:
  // http://doc.qt.nokia.com/4.7-snapshot/widgets-and-layouts.html
  QGridLayout *layout = new QGridLayout(this);
  QLabel *label = new QLabel(this);
  label->setText("Origin List");
  if (nofwd) {
    layout->addWidget(label, 0, 0);
    layout->addWidget(originSelect, 1, 0);
    layout->addWidget(peerInput, 2, 0);
  } else {
    layout->addWidget(label, 0, 1);
    layout->addWidget(originSelect, 1, 1);
    layout->addWidget(peerInput, 2, 1);
    layout->addWidget(tabs, 0, 0, -1, 1);
  }

  connect(peerInput, SIGNAL(returnPressed()),
	  this, SLOT(newPeer()));

  broadcast->focus();
}

ChatDialog::~ChatDialog() {
  delete peerInput;
  delete originSelect;
  delete broadcast;
  foreach(ChatTab *c, chats->values()) {
    delete c;
  }
  delete chats;
}

void ChatDialog::postMessage(QString name, QString msg, QColor color, QString dest) {
  if(dest.isEmpty()) {
    broadcast->postMessage(name, msg, color);
  } else {
    ChatTab *tab;
    if(chats->contains(dest))
      tab = chats->value(dest);
    else
      tab = newChatTab(dest);
    tab->postMessage(name, msg, color);
  }
}

void ChatDialog::newPeer() {
  QString peer = peerInput->text();
  peerInput->clear();
  emit addPeer(peer);
}

void ChatDialog::newOrigin(QString name) {
  originSelect->addItem(name);
}

ChatTab *ChatDialog::newChatTab(QString name) {
  ChatTab *tab = new ChatTab(name);
  chats->insert(name, tab);
  tabs->addTab(tab, name);
  connect(tab, SIGNAL(newMessage(QString, QString)),
	  this, SIGNAL(newMessage(QString, QString)));
  return tab;
}

void ChatDialog::openTab(QListWidgetItem *item) {
  ChatTab *tab;
  QString name = item->text();
  if(chats->contains(name))
    tab = chats->value(name);
  else
    tab = newChatTab(name);
  tabs->setCurrentWidget(tab);
  tab->focus();
}
