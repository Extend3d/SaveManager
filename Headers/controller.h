#ifndef CONTROLLER_H
#define CONTROLLER_H

#include <QThread>
#include <QNetworkAccessManager>

#include "redditinterface.h"
#include "localinterface.h"
#include "contentretriever.h"
#include "savedentry.h"

class Controller : public QObject
{
    Q_OBJECT

public:

    Controller();

    RedditInterface* redditInterface;
    LocalInterface* localInterface;

    QThread* start();

private:

    ContentRetriever* contentRetriever;

    QDir rootDir;
    QString clientID;
    QNetworkAccessManager* manager;

    void compareLists();
    void retrieveContent(QList<SavedEntry*>);

};

#endif // CONTROLLER_H
