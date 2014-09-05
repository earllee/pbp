#include <unistd.h>

#include <Peer.hh>
#include <QVarLengthArray>

Peer::Peer(QString n, QHostAddress h, quint16 p)
{
  name = n;
  host = h;
  port = p;
  messages = QVarLengthArray<QString>();
  need = 1;
}
