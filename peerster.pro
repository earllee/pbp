TEMPLATE = app
TARGET = 
DEPENDPATH += . src
INCLUDEPATH += . src
QT += network

# Input
HEADERS += src/NetSocket.hh src/ChatQTextEdit.hh src/ChatDialog.hh src/Peer.hh
SOURCES += main.cc src/NetSocket.cc src/ChatQTextEdit.cc src/ChatDialog.cc src/Peer.cc
