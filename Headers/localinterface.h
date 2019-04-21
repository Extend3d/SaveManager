#ifndef LOCALINTERFACE_H
#define LOCALINTERFACE_H

#include "savedentry.h"

#include <QDir>
#include <QFile>

class LocalInterface : public QObject
{
    Q_OBJECT

public:

    LocalInterface(QDir rootDir);

    void loadSavedList();
    QList<SavedEntry*> localSavedList;

private:

    QDir mediaDir;

private slots:

signals:

    void localSavedListReady();


};

#endif // LOCALINTERFACE_H
