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
#include <QMessageBox>
#include <QDebug>

ChatDialog::ChatDialog(bool nofwd) {
  setWindowTitle("Peerster");

  chats = new QMap<QString, ChatTab*>();
  colors = new QMap<QString, QColor>();

  peerInput = new QLineEdit(this);
  peerInput->setPlaceholderText("Add a peer");
  connect(peerInput, SIGNAL(returnPressed()),
	  this, SLOT(addPeer()));

  originSelect = new QListWidget(this);
  connect(originSelect, SIGNAL(itemDoubleClicked(QListWidgetItem*)),
	  this, SLOT(originClicked(QListWidgetItem*)));

  peerSelect = new QListWidget(this);
  connect(peerSelect, SIGNAL(itemDoubleClicked(QListWidgetItem*)),
	  this, SLOT(peerClicked(QListWidgetItem*)));

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
  QLabel *originLabel = new QLabel(this);
  originLabel->setText("Origin List");
  QLabel *peerLabel = new QLabel(this);
  peerLabel->setText("Peer List");
  if (nofwd) {
    layout->addWidget(originLabel, 0, 0);
    layout->addWidget(originSelect, 1, 0);
    layout->addWidget(peerInput, 2, 0);
  } else {
    layout->addWidget(tabs, 0, 0, 3, 1);
    layout->addWidget(originLabel, 0, 1);
    layout->addWidget(originSelect, 1, 1, 2, 1);
    layout->addWidget(peerLabel, 0, 2);
    layout->addWidget(peerSelect, 1, 2);
    layout->addWidget(peerInput, 2, 2);
    layout->addWidget(sharingBox, 3, 0, 1, -1);
  }

  broadcast->focus();
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

void ChatDialog::addPeer() {
  QString peer = peerInput->text();
  peerInput->clear();
  peerSelect->addItem(peer);
  setPeerState(peer, "Untrusted");
  emit addPeer(peer);
}

void ChatDialog::newOrigin(QString name) {
  originSelect->addItem(name);
  setOriginState(name, "Connected");
  // testing messageable
  messageable(name);
}

void ChatDialog::messageable(QString name) {
  setOriginState(name, "HaveKey");
  QMessageBox msgBox;
  msgBox.setText(QString("%1 has been added to your web of trust.").arg(name));
  msgBox.setStandardButtons(QMessageBox::Ok);
  msgBox.setDefaultButton(QMessageBox::Ok);
  msgBox.exec();
}

void ChatDialog::setOriginState(QString name, QString state) {
  QList<QListWidgetItem*> items = originSelect->findItems(name, Qt::MatchExactly);
  if (items.empty())
    originSelect->addItem(name);
  items = originSelect->findItems(name, Qt::MatchExactly);

  foreach(QListWidgetItem *item, items) {
    item->setData(Qt::UserRole, QVariant(state));
    if (state == "Connected") {
      item->setBackground(QBrush(QColor("#95A5A6")));
    } else if (state == "HaveKey") {
      item->setBackground(QBrush(QColor("#9B59B6")));
    } else if (state == "Rejected") {
      item->setBackground(QBrush(QColor("#E74C3C")));
    } else if (state == "Pending") {
      item->setBackground(QBrush(QColor("#3498DB")));
    } else if (state == "Friend") {
      item->setBackground(QBrush(QColor("#2ECC71")));
    }
  }
}

void ChatDialog::originClicked(QListWidgetItem *item) {
  QString state = item->data(Qt::UserRole).toString();
  QString name = item->text();
  QMessageBox msgBox;
  msgBox.setStandardButtons(QMessageBox::Yes | QMessageBox::No);
  msgBox.setDefaultButton(QMessageBox::No);
  if (state == "Connected") {
    msgBox.setText(QString("%1 is not in your web of trust.").arg(name));
    msgBox.setStandardButtons(QMessageBox::Ok);
    msgBox.setDefaultButton(QMessageBox::Ok);
  } else if (state == "HaveKey") {
    msgBox.setText(QString("Are you sure you want to send a friend request to this origin (%1)?").arg(name));
  } else if (state == "Pending") {
    msgBox.setText(QString("Are you sure you want to resend a friend request to this origin (%1)?").arg(name));
  } else if (state == "Rejected") {
    msgBox.setText(QString("Are you sure you want to accept this origin's (%1) friend request even though you rejected it earlier?").arg(name));
  } else if (state == "Friend") {
    ChatTab *tab;
    if (chats->contains(name))
      tab = chats->value(name);
    else
      tab = newChatTab(name);
    tabs->setCurrentWidget(tab);
    tab->focus();
    return;
  }

  switch (msgBox.exec()) {
  case QMessageBox::Yes:
    if (state == "HaveKey") {
      setOriginState(name, "Pending");
      emit requestFriend(name);
    } else if (state == "Pending") {
      emit requestFriend(name);
    } else if (state == "Rejected") {
      setOriginState(name, "Friend");
      emit friendApproved(name);
    }    
    break;
    // don't do anything if no
  }
}

void ChatDialog::approveFriend(QString name) {
  QMessageBox msgBox;
  msgBox.setStandardButtons(QMessageBox::Yes | QMessageBox::No);
  msgBox.setDefaultButton(QMessageBox::No);
  msgBox.setText(QString("%1 sent you a friend request. Accept?").arg(name));
  switch (msgBox.exec()) {
  case QMessageBox::Yes:
    setOriginState(name, "Friend");
    emit friendApproved(name);
    break;
  case QMessageBox::No:
    setOriginState(name, "Rejected");
    break;
  }
}

void ChatDialog::acceptedFriend(QString name) {
  setOriginState(name, "Friend");
  QMessageBox msgBox;
  msgBox.setText(QString("%1 accepted your friend request.").arg(name));
  msgBox.setStandardButtons(QMessageBox::Ok);
  msgBox.setDefaultButton(QMessageBox::Ok);
  msgBox.exec();
}

ChatTab *ChatDialog::newChatTab(QString name) {
  qDebug() << "again";
  ChatTab *tab = new ChatTab(name);
  chats->insert(name, tab);
  tabs->addTab(tab, name);
  connect(tab, SIGNAL(newMessage(QString, QString)),
	  this, SIGNAL(newMessage(QString, QString)));
  return tab;
}

void ChatDialog::newPeer(QString name) {
  peerSelect->addItem(name);
  setPeerState(name, "Untrusted");
}

void ChatDialog::peerClicked(QListWidgetItem *item) {
  QString state = item->data(Qt::UserRole).toString();
  QString peer = item->text();
  QMessageBox msgBox;
  msgBox.setStandardButtons(QMessageBox::Yes | QMessageBox::No);
  msgBox.setDefaultButton(QMessageBox::No);
  if (state == "Untrusted") {
    msgBox.setText(QString("Are you sure you want to send a trust request to this peer (%1)?").arg(peer));
  } else if (state == "Rejected") {
    msgBox.setText(QString("Are you sure you want to accept trust from this peer (%1) even though you rejected it earlier?").arg(peer));
  } else if (state == "Pending") {
    msgBox.setText(QString("Are you sure you want to resend a trust request to this peer (%1)?").arg(peer));
  } else if (state == "Trusted") {
    msgBox.setText(QString("You already trust this peer (%1).").arg(peer));
    msgBox.setStandardButtons(QMessageBox::Ok);
    msgBox.setDefaultButton(QMessageBox::Ok);
    msgBox.exec();
    return;
  }
  switch (msgBox.exec()) {
  case QMessageBox::Yes:
    if (state == "Untrusted") {
      setPeerState(peer, "Pending");
      emit requestTrust(peer);
    } else if (state == "Rejected") {
      setPeerState(peer, "Trusted");
      emit trustApproved(peer);
    } else if (state == "Pending") {
      setPeerState(peer, "Pending");
      emit requestTrust(peer);
    }    
    break;
    // don't do anything if no
  }
}

void ChatDialog::approveTrust(QString peer) {
  QMessageBox msgBox;
  msgBox.setStandardButtons(QMessageBox::Yes | QMessageBox::No);
  msgBox.setDefaultButton(QMessageBox::No);
  msgBox.setText(QString("%1 sent you a trust request. Accept?").arg(peer));
  switch (msgBox.exec()) {
  case QMessageBox::Yes:
    setPeerState(peer, "Trusted");
    emit trustApproved(peer);
    break;
  case QMessageBox::No:
    setPeerState(peer, "Rejected");
    break;
  }
}

void ChatDialog::acceptedTrust(QString peer) {
  setPeerState(peer, "Trusted");
  QMessageBox msgBox;
  msgBox.setText(QString("%1 accepted your trust request.").arg(peer));
  msgBox.setStandardButtons(QMessageBox::Ok);
  msgBox.setDefaultButton(QMessageBox::Ok);
  msgBox.exec();
}

void ChatDialog::setPeerState(QString peer, QString state) {
  QList<QListWidgetItem*> items = peerSelect->findItems(peer, Qt::MatchExactly);
  if (items.empty())
    peerSelect->addItem(peer);
  items = peerSelect->findItems(peer, Qt::MatchExactly);

  foreach(QListWidgetItem *item, items) {
    item->setData(Qt::UserRole, QVariant(state));
    if (state == "Untrusted") {
      item->setBackground(QBrush(QColor("#95A5A6")));
    } else if (state == "Rejected") {
      item->setBackground(QBrush(QColor("#E74C3C")));
    } else if (state == "Pending") {
      item->setBackground(QBrush(QColor("#3498DB")));
    } else if (state == "Trusted") {
      item->setBackground(QBrush(QColor("#2ECC71")));
    }
  }
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
