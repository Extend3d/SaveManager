#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include "controller.h"
#include "savedentry.h"

#include <QMainWindow>
#include <QDebug>
#include <QPushButton>
#include <QLayout>
#include <QLabel>
#include <QStackedLayout>
#include <QWidget>

namespace Ui {
class MainWindow;
}

class MainWindow : public QMainWindow
{
    Q_OBJECT

public:

    explicit MainWindow(QWidget *parent = nullptr);
    ~MainWindow();

    QWidget* createDelegate(QString, QString, QString, QString);

    Controller* contr;
    QThread* updateThread;

private slots:

    void onButtonClicked();
    void populateScrollArea();
    void on_actionBackup_Locally_triggered();

private:

    Ui::MainWindow *ui;
};

#endif // MAINWINDOW_H