TEMPLATE = app
TARGET = 
DEPENDPATH += . src
INCLUDEPATH += . src
QT += network

# Input
HEADERS += src/Origin.hh src/OriginList.hh src/Peer.hh src/PeerList.hh src/NetSocket.hh src/ChatDialog.hh src/ChatQTextEdit.hh src/ChatTab.hh
SOURCES += main.cc src/Origin.cc src/OriginList.cc src/Peer.cc src/PeerList.cc src/NetSocket.cc src/ChatDialog.cc src/ChatQTextEdit.cc src/ChatTab.cc
