#include <unistd.h>

#include <QGroupBox>
#include <QHBoxLayout>
#include <QLabel>
#include <QProgressBar>
#include <DownloadBox.hh>

DownloadBox::DownloadBox(QString name) {
  this->setFlat(true);

  label = new QLabel(name, this);
  progress = new QProgressBar(this);

  layout = new QHBoxLayout(this);
  layout->addWidget(label);
  layout->addWidget(progress);
}

DownloadBox::~DownloadBox() {
  delete progress;
  delete label;
  delete layout;
}

void DownloadBox::max(qint64 m) {
  progress->setMaximum(m);
}

void DownloadBox::update(qint64 v) {
  progress->setValue(v);
}
