#include "Headers/controller.h"

Controller::Controller()
{

    //TODO Figure out how the directory nonsense will work
    QDir rootDir = QDir::current();
    rootDir.cdUp();
    this->rootDir = rootDir;

    this->redditInterface = new RedditInterface();
    this->localInterface = new LocalInterface(rootDir);
    connect(redditInterface, &RedditInterface::loadingSavedList, localInterface, &LocalInterface::loadSavedList);

    this->manager = new QNetworkAccessManager();
    this->contentRetriever = new ContentRetriever(manager, rootDir);

}

QThread* Controller::start()
{
    qDebug() << "Controller Thread: " << QThread::currentThread();

    //Create new thread and move the redditInterface object to that thread
    QThread* t1 = new QThread();
    redditInterface->moveToThread(t1);

    //Connect the QThread::started signal to the starting point of the redditInterface so things are set in motion
    //when t1->start() is called
    connect(t1, &QThread::started, redditInterface, &RedditInterface::loadSavedList);

    //Every set duration, redditInterface updates the saved list. When the list is updated, Controller::compareLists checks
    //to see if any of the items in the list need to be downloaded
    connect(redditInterface,&RedditInterface::savedListReady, this, &Controller::compareLists);

    //Start of the program
    t1->start();

    return t1;

}

void Controller::compareLists()
{

    //Get handles on both the new saved list from reddit and the list of what is already saved locally
    QList<SavedEntry*> redditSavedList = redditInterface->savedList;
    QList<SavedEntry*> localSavedList = localInterface->savedList;

    QString localSavedListPath = rootDir.path() + "/media/localSavedList.txt";

    QFile f(localSavedListPath);
    if (!f.open(QIODevice::Append)) {
        qFatal("Failed to open local saved list");
    }
    QTextStream stream(&f);

    //Iterate through the posts from reddit
    QList<SavedEntry*> postsToRetrieve;
    for (int i = 0; i < redditSavedList.length(); i++) {

        SavedEntry* entry = redditSavedList.at(i);
        QString entryID = entry->idNum;
        bool isLocal = false;

        //Check the local saved lists to see if its already local
        for (int j = 0; i < localSavedList.length(); j++) {
            if (localSavedList.at(j)->idNum == entryID) {
                isLocal = true;
                break;
            }
        }
        //If the post hasn't been saved locally, save it
        if (!isLocal) {

            postsToRetrieve.append(entry);

            //Local saved entries take paramters: type, text, url, idNum
            QString out = QString("%1|||%2|||%3|||%4||||").arg(entry->type, entry->text, entry->url, entry->idNum);
            stream << out.toUtf8();

        }
    }
    f.flush();
    f.close();

    qDebug() << "Number of posts to be downloaded: " << postsToRetrieve.length();
    retrieveContent(postsToRetrieve);
}

void Controller::retrieveContent(QList<SavedEntry*> content)
{

    for (int i = 0; i < content.length(); i++) {

        contentRetriever->retrieve(content.at(i));

    }
}


