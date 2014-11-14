#ifndef DOWNLOADBOX_HH
#define DOWNLOADBOX_HH

#include <QGroupBox>
#include <QHBoxLayout>
#include <QLabel>
#include <QProgressBar>

class DownloadBox : public QGroupBox {
  Q_OBJECT

public:
  DownloadBox(QString);
  ~DownloadBox();
  void update(qint64);
  void max(qint64);
private:
  QProgressBar *progress;
  QLabel *label;
  QHBoxLayout *layout;
};

#endif
