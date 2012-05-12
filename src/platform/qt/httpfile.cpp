// This file is part of SmallBASIC
//
// Copyright(C) 2001-2011 Chris Warren-Smith. [http://tinyurl.com/ja2ss]
//
// This program is distributed under the terms of the GPL v2.0 or later
// Download the GNU Public License (GPL) from www.gnu.org
//

#include <QNetworkAccessManager>
#include "httpfile.h"

QNetworkAccessManager manager;

HttpFile::HttpFile(HttpFileListener *listener, const QString path) : 
  QTemporaryFile() {
  this->listener = listener;
  this->url = QUrl(path);
  requestFile();
}

HttpFile::~HttpFile() {
  reply->deleteLater();
}

void HttpFile::requestFile() {
  QNetworkRequest request;
  request.setUrl(url);
  request.setRawHeader("User-Agent", "SmallBASIC - QT");

  open(QIODevice::WriteOnly);
  reply = manager.get(request);
  connect(reply, SIGNAL(readyRead()), this, SLOT(readyRead()));
  connect(reply, SIGNAL(finished()), this, SLOT(finished()));
}

void HttpFile::finished() {
  flush();
  close();

  QVariant redirectionTarget = reply->attribute(QNetworkRequest::RedirectionTargetAttribute);
  if (reply->error()) {
    listener->loadError(reply->errorString());
    deleteLater();
  } else if (!redirectionTarget.isNull()) {
    url = url.resolved(redirectionTarget.toUrl());
    open(QIODevice::WriteOnly);
    resize(0);
    requestFile();
  } else {
    // success
    listener->loadPath(fileName(), false, true);
    deleteLater();
  }
}

// Called every time the QNetworkReply has new data. Read all of
// its new data and write it into the  This uses less RAM than
// when reading it at the finished() signal of the QNetworkReply
void HttpFile::readyRead() {
  write(reply->readAll());
}
