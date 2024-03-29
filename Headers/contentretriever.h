#ifndef CONTENTRETRIEVER_H
#define CONTENTRETRIEVER_H

#include "savedentry.h"

#include <QtNetwork>

class ContentRetriever : public QObject
{
    Q_OBJECT

public:

    ContentRetriever(QNetworkAccessManager* manager, QDir rootDir);

    QDir mediaDir;
    QString clientID;

    QNetworkAccessManager* manager;

    void retrieve(SavedEntry*);
    void download(QString url, QString idNum);

    void processResponse(QNetworkReply*);


};

#endif // CONTENTRETRIEVER_H
