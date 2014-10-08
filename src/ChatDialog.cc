#include <unistd.h>

#include <ChatDialog.hh>
#include <ChatTab.hh>
#include <SharedFile.hh>
#include <QColor>
#include <QFont>
#include <QGroupBox>
#include <QGridLayout>
#include <QLabel>
#include <QPushButton>
#include <QFileDialog>
#include <QByteArray>
#include <QDebug>

ChatDialog::ChatDialog(bool nofwd) {
  setWindowTitle("Peerster");

  chats = new QMap<QString, ChatTab*>();

  peerInput = new QLineEdit(this);
  peerInput->setPlaceholderText("Add a peer");
  connect(peerInput, SIGNAL(returnPressed()),
	  this, SLOT(newPeer()));

  originSelect = new QListWidget(this);
  connect(originSelect, SIGNAL(itemDoubleClicked(QListWidgetItem*)),
	  this, SLOT(openTab(QListWidgetItem*)));

  tabs = new QTabWidget();
  broadcast = new ChatTab("");
  tabs->addTab(broadcast, "Broadcast");
  connect(broadcast, SIGNAL(newMessage(QString, QString)),
	  this, SIGNAL(newMessage(QString, QString)));

  sharingBox = new QGroupBox(this);
  sharingBox->setFlat(true);
  sharingButton = new QPushButton("Share file(s)", sharingBox);
  connect(sharingButton, SIGNAL(clicked()),
	  this, SLOT(openFileDialog()));
  sharingInput = new QLineEdit(sharingBox);
  sharingInput->setPlaceholderText("Search for files");
  connect(sharingInput, SIGNAL(returnPressed()),
	  this, SLOT(initiateSearch()));
  sharingSearch = new QPushButton("Search", sharingBox);
  connect(sharingSearch, SIGNAL(clicked()),
	  this, SLOT(initiateSearch()));
  sharingResults = new QListWidget(sharingBox);
  connect(sharingResults, SIGNAL(itemDoubleClicked(QListWidgetItem*)),
	  this, SLOT(startDownload(QListWidgetItem*)));
  sharingLayout = new QGridLayout(sharingBox);
  sharingLayout->addWidget(sharingButton, 0, 0, 1, -1);
  sharingLayout->addWidget(sharingInput, 1, 0, 1, 2);
  sharingLayout->addWidget(sharingSearch, 1, 2, 1, 1);
  sharingLayout->addWidget(sharingResults, 2, 0, 1, -1);
  
  results = new QMap<QString, QVariantMap>();
  // Lay out the widgets to appear in the main window.
  // For Qt widget and layout concepts see:
  // http://doc.qt.nokia.com/4.7-snapshot/widgets-and-layouts.html
  layout = new QGridLayout(this);
  QLabel *label = new QLabel(this);
  label->setText("Origin List");
  if (nofwd) {
    layout->addWidget(label, 0, 0);
    layout->addWidget(originSelect, 1, 0);
    layout->addWidget(peerInput, 2, 0);
  } else {
    layout->addWidget(tabs, 0, 0, 3, 1);
    layout->addWidget(label, 0, 1);
    layout->addWidget(originSelect, 1, 1);
    layout->addWidget(peerInput, 2, 1);
    layout->addWidget(sharingBox, 3, 0, 1, -1);
  }

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

void ChatDialog::openFileDialog() {
  QFileDialog dialog(this);
  dialog.setFileMode(QFileDialog::ExistingFiles);
  if (dialog.exec()) {
    foreach (QString filename, dialog.selectedFiles()) {
      emit shareFile(filename);
    }
  }
}

void ChatDialog::initiateSearch() {
  QString query = sharingInput->text();
  if (query.isEmpty())
    return;
  sharingInput->clear();
  results->clear();
  sharingResults->clear();
  emit search(query);
}

void ChatDialog::searchReply(QVariantMap reply) {
  QString key = QString("%1 (%2)")
    .arg(reply.value("Filename").toString())
    .arg(reply.value("Origin").toString());
  results->insert(key, reply);
  sharingResults->addItem(key);
}

void ChatDialog::startDownload(QListWidgetItem *item) {
  QVariantMap reply = results->value(item->text());
  emit downloadFile(reply.value("Filename").toString(),
		    reply.value("ID").toByteArray(),
		    reply.value("Origin").toString());
}
