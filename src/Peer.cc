#include <unistd.h>

#include <Peer.hh>
#include <QVector>

Peer::Peer(QString n, QHostAddress h, quint16 p)
{
  name = n;
  host = h;
  port = p;
  messages = QVector<QString>();
  need = 1;
}

quint32 Peer::Next()
{
  return need;
}

QString Peer::Get(quint32 seqNo)
{
  return messages.value(seqNo - 1);
}

void Peer::Add(quint32 seqNo, QString text)
{
  if((int)seqNo > messages.size()) {
    messages.resize(seqNo);
  }
  if(messages.value(seqNo - 1) == "" && seqNo >= need) {
    // set the new message if it doesn't already exist
    messages[seqNo - 1] = text;
    if(seqNo == need) {
      // if the seqNo is the one that is needed then increment need until
      // the first missing message (in case later messages were received
      // first)
      for(; messages.value(need - 1) != ""; need++) {
	// emit all messages that were waiting
	emit chatMessage(name, messages.value(need - 1));
      }
    }
  }
}
