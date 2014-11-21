TEMPLATE = app
TARGET = 
DEPENDPATH += . src
INCLUDEPATH += . src
QT += network
CONFIG += crypto

# Input
HEADERS += src/Origin.hh src/Peer.hh src/PeerList.hh src/NetSocket.hh src/ChatDialog.hh src/ChatQTextEdit.hh src/ChatTab.hh src/SharedFile.hh src/DownloadBox.hh src/PBP.hh
SOURCES += main.cc src/Origin.cc src/Peer.cc src/PeerList.cc src/NetSocket.cc src/ChatDialog.cc src/ChatQTextEdit.cc src/ChatTab.cc src/SharedFile.cc src/DownloadBox.cc src/PBP.cc
