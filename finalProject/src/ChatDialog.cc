#include <unistd.h>

#include <ChatDialog.hh>
#include <DownloadBox.hh>
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
  colors = new QMap<QString, QColor>();

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
  sharingButton->setDefault(false);
  sharingButton->setAutoDefault(false);
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
  sharingFiles = new QListWidget(sharingBox);
  sharingLayout = new QGridLayout(sharingBox);
  sharingLayout->addWidget(sharingInput, 0, 0, 1, 2);
  sharingLayout->addWidget(sharingSearch, 0, 2, 1, 1);
  sharingLayout->addWidget(sharingButton, 0, 3, 1, 2);
  sharingLayout->addWidget(sharingResults, 1, 0, 1, 3);
  sharingLayout->addWidget(sharingFiles, 1, 3, 1, 2);
  downloads = new QMap<QByteArray, DownloadBox*>();
  
  results = new QMap<QByteArray, QPair<QString, QString> >();
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
  delete broadcast;
  delete peerInput;
  delete originSelect;
  foreach(ChatTab *c, chats->values()) {
    delete c;
  }
  delete chats;

  delete tabs;
  delete sharingButton;
  delete sharingSearch;
  delete sharingInput;
  delete sharingResults;
  delete sharingFiles;
  delete sharingLayout;
  delete sharingBox;
  delete layout;
  delete results;
  delete colors;
  foreach(DownloadBox *d, downloads->values()) {
    delete d;
  }
  delete downloads;
}

void ChatDialog::postMessage(QString name, QString msg, QString dest) {
  QColor color;
  if (!colors->contains(name)) {
    color.setHsv(qrand() % 360, 128 + qrand() % 128, (qrand() % 128) + 64);
    colors->insert(name, color);
  } else {
    color = colors->value(name);
  }

  // determine which tab to send it to
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
      sharingFiles->addItem(filename);
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

void ChatDialog::searchReply(QByteArray id, QString filename, QString origin) {
  results->insert(id, QPair<QString, QString>(filename, origin));
  sharingResults->addItem(filename);
  QListWidgetItem *widget = sharingResults->item(sharingResults->count() - 1);
  widget->setData(Qt::UserRole, QVariant(id));
}

void ChatDialog::startDownload(QListWidgetItem *item) {
  QByteArray id = item->data(Qt::UserRole).toByteArray();
  QPair<QString, QString> info = results->value(id);
  DownloadBox *d = new DownloadBox(info.first);
  downloads->insert(id, d);
  sharingLayout->addWidget(d, sharingLayout->rowCount(), 0, 1, -1);
  emit downloadFile(id,
		    info.first,
		    info.second);
}

void ChatDialog::receivedBlocklist(QByteArray id, qint64 block) {
  downloads->value(id)->max(block);
}

void ChatDialog::receivedBlock(QByteArray id, qint64 block) {
  downloads->value(id)->update(block);
}
