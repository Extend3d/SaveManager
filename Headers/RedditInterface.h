#ifndef REDDITINTERFACE_H
#define REDDITINTERFACE_H

#include "savedentry.h"

#include <QtNetwork>
#include <QJsonObject>


class RedditInterface : public QObject
{
    Q_OBJECT

public:

    RedditInterface();

    void loadSavedList();
    QList<SavedEntry*> savedList;

    enum State { Running, Asleep };
    State state;

    QWaitCondition* waitCondition;


private:

    void buildSavedList();

    void sleep();

    QString userJSONFeedUrl;
    QNetworkAccessManager* manager;
    QList<QString> jsonPages;


private slots:

    void requestJSONPage(QString);
    void retrieveJSONPage(QNetworkReply*);

signals:

    void requestNext(QString);
    void processJSON();

    void savedListReady();

    void finished();
    void doneSleeping();

};

#endif // REDDITINTERFACE_H
