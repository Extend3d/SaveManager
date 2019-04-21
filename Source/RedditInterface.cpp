#include "Headers/RedditInterface.h"


RedditInterface::RedditInterface()
{
    this->userJSONFeedUrl = "https://www.reddit.com/user/TheQuirks/saved.json?feed=d16e81d0dd79361e64528e39b073616776b799ec&user=TheQuirks";
    this->manager = new QNetworkAccessManager(this);

    //These connects make the reddit interface class request a new saved list from the userJSONFeedUrl every set amount of seconds
    this->waitCondition =  new QWaitCondition;
    connect(this, &RedditInterface::finished, this, &RedditInterface::sleep);
    connect(this, &RedditInterface::doneSleeping, this, &RedditInterface::loadSavedList);

    //These connects go around in a loop, requesting and waiting for each page of the users saved data (Each page contains max 100 posts)
    connect(manager, &QNetworkAccessManager::finished, this, &RedditInterface::retrieveJSONPage);
    connect(this, &RedditInterface::requestNext, this, &RedditInterface::requestJSONPage);
    connect(this, &RedditInterface::processJSON, this, &RedditInterface::buildSavedList);

}

void RedditInterface::sleep()
{

    QMutex mutex;
    mutex.lock();
    this->state = Asleep;
    this->waitCondition->wait(&mutex, 60000);
    this->state = Running;
    mutex.unlock();
    emit doneSleeping();

}


void RedditInterface::loadSavedList()
{
    qDebug() << "REQUESTING NEW SAVED LIST" << QThread::currentThread();

    this->savedList.clear();
    this->jsonPages.clear();

    requestJSONPage("");
}

void RedditInterface::requestJSONPage(QString lastPost)
{
    //HTTP request, "application/x-www-form-urlencoded" is just a default of some kind
    QNetworkRequest request(QUrl(this->userJSONFeedUrl + "&limit=100" + "&after=" + lastPost));
    request.setHeader(QNetworkRequest::ContentTypeHeader, "application/x-www-form-urlencoded");

    manager->get(request);
}

void RedditInterface::retrieveJSONPage(QNetworkReply* reply)
{
    int numPosts = 0;

    QByteArray bytes = reply->readAll();
    int statusCode = reply->attribute(QNetworkRequest::HttpStatusCodeAttribute).toInt();

    if(statusCode == 200) {

        QString str = QString::fromUtf8(bytes.data(), bytes.size());
        jsonPages.append(str);

        int postsRecieved = str.split("\"dist\": ")[1].split(",")[0].toInt();
        numPosts += postsRecieved;

        if (str.contains("\"after\": \"")) {

            QString lastPost = str.split("\"after\": \"")[1].split("\",")[0];
            qDebug() << QString::number(numPosts) << "posts recieved." << lastPost;
            emit requestNext(lastPost);
        }
        else {
            qDebug() << QString::number(numPosts) << "posts recieved." << "end";
            emit processJSON();
        }
    }
    else {
        qCritical() << "Failed to retrieve JSON page.\nStatus Code:" << statusCode;
    }
}

void RedditInterface::buildSavedList()
{

    QList<QJsonObject> pages;
    for (int i = 0; i < jsonPages.length(); i++) {
        pages.append(QJsonDocument::fromJson(jsonPages.at(i).toUtf8()).object());
    }

    for (int i = 0; i < pages.length(); i++) {

         int numChildren = pages.at(i)["data"]["dist"].toInt();

         for (int j = 0; j < numChildren; j++) {

            QString type = pages.at(i)["data"]["children"][j]["kind"].toString();
            QString title;
            QString url;
            QString idNum = pages.at(i)["data"]["children"][j]["data"]["id"].toString();

            if (type == "t3") {
                title = pages.at(i)["data"]["children"][j]["data"]["title"].toString();
                url = pages.at(i)["data"]["children"][j]["data"]["url"].toString();
            }
            else if (type == "t1") {
                title = pages.at(i)["data"]["children"][j]["data"]["body"].toString();
                url = "https://www.reddit.com" + pages.at(i)["data"]["children"][j]["data"]["permalink"].toString();
            }
            else {
                qDebug() << "NEW TYPE" << type;
            }
            this->savedList.append(new SavedEntry(type, title, url, idNum));
         }
    }

    qDebug() << "Length of savedList:" << savedList.length();
    emit savedListReady();
    emit finished();

}
