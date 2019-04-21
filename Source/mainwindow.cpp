#include "Headers/mainwindow.h"
#include "ui_mainwindow.h"


MainWindow::MainWindow(QWidget *parent) :
    QMainWindow(parent),
    ui(new Ui::MainWindow)
{
    ui->setupUi(this);

    this->contr = new Controller();
    this->updateThread = nullptr;



}

MainWindow::~MainWindow()
{
    delete ui;
}

void MainWindow::onButtonClicked()
{

    qDebug() << sender()->parent()->objectName();

}

void MainWindow::populateScrollArea()
{

//    //Setup scrollArea
//    QWidget* bigContainer = new QWidget(this);
//    QVBoxLayout* vertical = new QVBoxLayout;
//    bigContainer->setLayout(vertical);
//    //scrollArea


//    //Populate scrollArea
//    for (int i = 0; i < redditInterface->savedList.length(); i++) {

//        QString type = redditInterface->savedList.at(i)->type;
//        QString text = redditInterface->savedList.at(i)->text;
//        QString url = redditInterface->savedList.at(i)->url;
//        QString idNum = redditInterface->savedList.at(i)->idNum;

//        QWidget* delegate = createDelegate(type, text, url, idNum);

//        vertical->addWidget(delegate);

//    } //for

//    //Generate "Load More" button at bottom of list
//    QWidget* finalDelegate = new QWidget;
//    QPushButton* loadMore = new QPushButton("loadMore");
//    QHBoxLayout* horizontal = new QHBoxLayout;
//    horizontal->addWidget(loadMore);
//    finalDelegate->setLayout(horizontal);
//    vertical->addWidget(finalDelegate);
//    //Load More

//    ui->scrollArea->setWidget(bigContainer);

}
QWidget* MainWindow::createDelegate(QString type, QString text, QString url, QString idNum) {

    QWidget* delegate = new QWidget;
    delegate->setObjectName("Delegate " + idNum);
    delegate->setProperty("type", type);
    delegate->setProperty("text", text);
    delegate->setProperty("url", url);
    delegate->setProperty("idNum", idNum);

    QLabel* label = new QLabel(text);
    QLineEdit* tags = new QLineEdit(url);

    QPushButton* button = new QPushButton(delegate);
    button->setText(type);


    QVBoxLayout* vertical = new QVBoxLayout;

    QWidget* container = new QWidget;
    QHBoxLayout* horizontal = new QHBoxLayout;
    horizontal->addWidget(button);
    horizontal->addWidget(label);
    container->setLayout(horizontal);

    vertical->addWidget(container);
    vertical->addWidget(tags);

    delegate->setLayout(vertical);

    connect(button, &QPushButton::pressed, this,  &MainWindow::onButtonClicked);

    return delegate;
}

void MainWindow::on_actionBackup_Locally_triggered()
{

    if (updateThread == nullptr) {
        this->updateThread = contr->start();
    }
    else if (contr->redditInterface->state == RedditInterface::Running) {
        qDebug() << "Content already being downloaded";
    }
    else if (contr->redditInterface->state == RedditInterface::Asleep) {
        contr->redditInterface->waitCondition->wakeOne();
    }

}
