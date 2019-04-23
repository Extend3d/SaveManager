#include "Headers/mainwindow.h"
#include "ui_mainwindow.h"




MainWindow::MainWindow(QWidget *parent) :
    QMainWindow(parent),
    ui(new Ui::MainWindow)
{
    ui->setupUi(this);

    this->contr = new Controller();
    this->localInterface = contr->localInterface;
    connect(localInterface, &LocalInterface::localSavedListReady, this, &MainWindow::populateScrollArea);
    this->updateThread = nullptr;

    connect(ui->lineEdit, &QLineEdit::returnPressed, this, &MainWindow::searchScrollArea);

    QStackedLayout* stack = new QStackedLayout();
    QSizePolicy policy(QSizePolicy::Expanding, QSizePolicy::Expanding);

    QWidget* mediaContainer = new QWidget();
    mediaContainer->setObjectName("mediaContainer");
    ui->horizontalLayout_2->addWidget(mediaContainer);
    mediaContainer->setLayout(stack);
    mediaContainer->setSizePolicy(policy);

    imageHolder = new QLabel();
    imageHolder->setObjectName("imageHolder");
    imageHolder->setParent(mediaContainer);
    mediaContainer->layout()->addWidget(imageHolder);

}

MainWindow::~MainWindow()
{
    delete ui;
}

void MainWindow::onButtonClicked()
{

    QString idNum = sender()->parent()->parent()->property("idNum").toString();

    QString file = localInterface->locateFile(idNum);
    if (file != nullptr) {

        if (file.contains(".jp") || file.contains(".png")) {
            QPixmap pixmap(file);

            int w = imageHolder->width();
            int h = imageHolder->height();

            imageHolder->setPixmap(pixmap.scaled(w, h, Qt::KeepAspectRatio));
        }
    }
}

void MainWindow::populateScrollArea()
{

    //Setup scrollArea
    QWidget* bigContainer = new QWidget(this);
    QVBoxLayout* vertical = new QVBoxLayout;
    vertical->setObjectName("delegateContainer");
    bigContainer->setLayout(vertical);
    //scrollArea

    //Populate scrollArea
    for (int i = 0; i < localInterface->savedList.length(); i++) {

        QString type = localInterface->savedList.at(i)->type;
        QString text = localInterface->savedList.at(i)->text;
        QString url = localInterface->savedList.at(i)->url;
        QString idNum = localInterface->savedList.at(i)->idNum;

        QWidget* delegate = createDelegate(type, text, url, idNum);
        delegateList.append(delegate);

        vertical->addWidget(delegate);
        vertical->addStrut(5);

    }
    ui->scrollArea->setWidget(bigContainer);

}
QWidget* MainWindow::createDelegate(QString type, QString text, QString url, QString idNum) {

    QWidget* delegate = new QWidget;
    delegate->setObjectName("Delegate " + idNum);
    delegate->setProperty("type", type);
    delegate->setProperty("text", text);
    delegate->setProperty("url", url);
    delegate->setProperty("idNum", idNum);
    delegate->setMaximumWidth(ui->scrollArea->width() - 50);


    if (text.length() >= 40) { text.truncate(40); text += "..."; }
    QLabel* label = new QLabel(text);
    QLineEdit* tags = new QLineEdit(url);

    QPushButton* button = new QPushButton();
    button->setText(type);
    button->setMaximumWidth(100);
    button->setParent(delegate);


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

    localInterface->loadSavedList();
    return;

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

void MainWindow::searchScrollArea() {

    QString search = ui->lineEdit->text();

    for (int i = 0; i < delegateList.size(); i++) {

        QWidget* delegate = delegateList.at(i);

        QString type = delegate->property("type").toString();
        QString text = delegate->property("text").toString();
        QString url = delegate->property("url").toString();
        QString idNum = delegate->property("idNum").toString();

        if (!(type.contains(search) || text.contains(search) || url.contains(search) || idNum.contains(search))) {
            delegate->hide();
        }
        else if (type.contains(search) || text.contains(search) || url.contains(search) || idNum.contains(search)) {
            delegate->show();
        }


    }
}

