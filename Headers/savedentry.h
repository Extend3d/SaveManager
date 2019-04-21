#ifndef SAVEDENTRY_H
#define SAVEDENTRY_H

#include <QString>
#include <QDebug>

class SavedEntry
{

public:

    QString type;
    QString text;
    QString url;
    QString idNum;


public:

    SavedEntry(QString, QString, QString, QString);

};

#endif // SAVEDENTRY_H
