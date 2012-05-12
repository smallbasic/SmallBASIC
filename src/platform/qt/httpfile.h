// This file is part of SmallBASIC
//
// Copyright(C) 2001-2011 Chris Warren-Smith. [http://tinyurl.com/ja2ss]
//
// This program is distributed under the terms of the GPL v2.0 or later
// Download the GNU Public License (GPL) from www.gnu.org
//

#ifndef HTTPFILE_H
#define HTTPFILE_H

#include <QNetworkReply>
#include <QNetworkRequest>
#include <QTemporaryFile>

struct HttpFileListener {
  virtual void loadPath(QString path, bool showPath, bool setHistory) = 0;
  virtual void loadError(QString message) = 0;
};

class HttpFile : public QTemporaryFile {
  Q_OBJECT

public:
  HttpFile(HttpFileListener* listener, const QString path);
  ~HttpFile();

public slots:
  void finished();
  void readyRead();

private:
  void requestFile();

  QUrl url;
  QNetworkReply *reply;
  HttpFileListener *listener;
};

#endif
