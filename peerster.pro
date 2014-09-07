TEMPLATE = app
TARGET = 
DEPENDPATH += . src
INCLUDEPATH += . src
QT += network

# Input
# HEADERS += src/NetSocket.hh src/ChatQTextEdit.hh src/ChatDialog.hh src/Peer.hh src/PeerConnection.hh
# SOURCES += main.cc src/NetSocket.cc src/ChatQTextEdit.cc src/ChatDialog.cc src/Peer.cc src/PeerConnection.cc
HEADERS += src/Origin.hh
SOURCES += main.cc src/Origin.cc
