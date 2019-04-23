#include "Headers/localinterface.h"


LocalInterface::LocalInterface(QDir rootDir)
{

    if (!rootDir.exists("media")) {
        qDebug() << "<Media directory does not exist>\nCreating new directory...\n";
        rootDir.mkdir("media");
    }

    rootDir.cd("media");
    this->mediaDir = rootDir;

}



void LocalInterface::loadSavedList()
{
    this->savedList.clear();

    QString line;
    QStringList params;

    QFile file(mediaDir.absolutePath() + "/localSavedList.txt");

    if(file.open(QIODevice::ReadOnly | QIODevice::Text)) {

        QTextStream stream(&file);
        QStringList entriesText = stream.readAll().split("||||");
        entriesText.removeAt(entriesText.length() - 1);

        if (entriesText.length() != 1) {

            foreach (QString entryText, entriesText) {

                params = entryText.split("|||");
                assert(params.length() == 4);

                //Local saved entries take paramters: type, text, url, idNum, and path
                savedList.append(new SavedEntry(params.at(0), params.at(1), params.at(2), params.at(3)));
            }
        }
    }
    file.close();

    qDebug() << "Length of localSavedList:" << savedList.length();
    emit localSavedListReady();
}

QString LocalInterface::locateFile(QString idNum) {

    QStringList files = mediaDir.entryList();
    for (int i = 0; i < files.length(); i++) {

        if (files.at(i).split(".")[0] == idNum) {
            return mediaDir.filePath(files.at(i));
        }

    }
    return nullptr;
}
