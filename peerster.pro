TEMPLATE = app
TARGET = 
DEPENDPATH += . src
INCLUDEPATH += . src
QT += network

# Input
HEADERS += src/Origin.hh src/OriginList.hh src/Peer.hh
SOURCES += main.cc src/Origin.cc src/OriginList.cc src/Peer.cc
