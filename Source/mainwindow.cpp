#include "Headers/mainwindow.h"
#include "ui_mainwindow.h"




MainWindow::MainWindow(QWidget *parent) :
    QMainWindow(parent),
    ui(new Ui::MainWindow)
{
    ui->setupUi(this);

    setWindowTitle("Save Manager");

    this->contr = new Controller();
    this->updateThread = nullptr;
    this->localInterface = contr->localInterface;
    connect(contr->redditInterface, &RedditInterface::savedListReady, this, &MainWindow::populateScrollArea);

    connect(ui->lineEdit, &QLineEdit::returnPressed, this, &MainWindow::searchScrollArea);

    ui->scrollArea->setFixedWidth(500);
    ui->lineEdit->setFixedWidth(500);
    ui->lineEdit->setPlaceholderText("Search...");

    //Widget that holds the stack of display widgets (below)
    QWidget* mediaContainer = new QWidget(this);
    mediaContainer->setObjectName("mediaContainer");
    ui->horizontalLayout_2->addWidget(mediaContainer);
    stack = new QStackedLayout();
    mediaContainer->setLayout(stack);
    QSizePolicy policy(QSizePolicy::Expanding, QSizePolicy::Expanding);
    mediaContainer->setSizePolicy(policy);

    //Widget to display static images
    imageHolder = new QLabel(mediaContainer);
    imageHolder->setSizePolicy(policy);
    mediaContainer->layout()->addWidget(imageHolder);

    //Widget to display webpage html
    htmlHolder = new QWebEngineView(mediaContainer);
    connect(htmlHolder, &QWebEngineView::loadFinished, this, [this](bool success) { if (success) stack->setCurrentWidget(htmlHolder); });
    mediaContainer->layout()->addWidget(htmlHolder);

    //Widget to display videos
    videoHolder = new QVideoWidget(mediaContainer);
    videoPlayer = new QMediaPlayer(mediaContainer);
    videoPlayer->setVideoOutput(videoHolder);
    //Simple connect to a lamda function to make sure the video plays on repeat, since apparently QMediaPlayer can't do that itself
    connect(videoPlayer, &QMediaPlayer::stateChanged, this, [this](QMediaPlayer::State state) {
        if (state == QMediaPlayer::StoppedState)
        {
           videoPlayer->setPosition(0);
           videoPlayer->play();
        }
    });
    mediaContainer->layout()->addWidget(videoHolder);

}

MainWindow::~MainWindow()
{
    delete ui;
}

void MainWindow::onButtonClicked()
{

    QString idNum = sender()->parent()->parent()->property("idNum").toString();

    QString file = localInterface->locateFile(idNum);
    qDebug() << file;
    if (file != nullptr) {

        if (file.contains(".jpeg") || file.contains(".png")) {

            QPixmap pixmap(file);
            int w = imageHolder->width();
            int h = imageHolder->height();
            imageHolder->setPixmap(pixmap.scaled(w, h, Qt::KeepAspectRatio));
            stack->setCurrentWidget(imageHolder);

        }
        else if (file.contains("html")) {

            //TODO fix html render issues
            QFile f(file);
            if (!f.open(QIODevice::ReadOnly)) {return;};
            QByteArray bytes = f.readAll();
            QString html = QString::fromUtf8(bytes, bytes.size());
            if (!(htmlHolder->property("idNum").toString() == idNum))
            {
                htmlHolder->setHtml(html);
                htmlHolder->setProperty("idNum", idNum);
            }
            //htmlHolder is set as current widget via the lambda function in the connnect statement above

        }
        else if (file.contains(".mp4"))
        {

                videoPlayer->setMedia(QUrl::fromLocalFile(file));
                videoPlayer->play();
                stack->setCurrentWidget(videoHolder);
        }
    }
}

void MainWindow::changeEvent(QEvent* e) {

    QMainWindow::changeEvent(e);
//    if (e->type() == QEvent::WindowStateChange && windowState() & Qt::WindowMinimized)
//        QMetaObject::invokeMethod(this, "hide", Qt::QueuedConnection);


}

void MainWindow::populateScrollArea()
{

    if (delegateList.length() > 0) {

        assert(this->findChild<QWidget*>("scrollAreaContainer") != nullptr);
        delete this->findChild<QWidget*>("scrollAreaContainer");

    }

    //Setup scrollArea
    QWidget* scrollAreaContainer = new QWidget(ui->scrollArea);
    scrollAreaContainer->setObjectName("scrollAreaContainer");
    //scrollAreaContainer->setMinimumSize(ui->scrollArea->size());
    QSizePolicy policy(QSizePolicy::Minimum, QSizePolicy::Minimum);
    scrollAreaContainer->setSizePolicy(policy);
    QVBoxLayout* vertical = new QVBoxLayout;
    vertical->setObjectName("delegateContainer");
    scrollAreaContainer->setLayout(vertical);
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
    ui->scrollArea->setWidget(scrollAreaContainer);
    ui->scrollArea->update();


}
QWidget* MainWindow::createDelegate(QString type, QString text, QString url, QString idNum) {

    QWidget* delegate = new QWidget;
    delegate->setMaximumWidth(500);
    QSizePolicy policy1(QSizePolicy::MinimumExpanding, QSizePolicy::Minimum);
    delegate->setSizePolicy(policy1);
    delegate->setObjectName("Delegate " + idNum);
    delegate->setProperty("type", type);
    delegate->setProperty("text", text);
    delegate->setProperty("url", url);
    delegate->setProperty("idNum", idNum);

    if (text.length() >= 45) { text.truncate(45); text += "..."; }
    QLabel* label = new QLabel(text);
    QSizePolicy policy2(QSizePolicy::Minimum, QSizePolicy::Minimum);
    label->setSizePolicy(policy2);

    QPushButton* button = new QPushButton();
    button->setFixedWidth(100);
    button->setText(idNum);
    button->setParent(delegate);
    connect(button, &QPushButton::pressed, this,  &MainWindow::onButtonClicked);

    QWidget* container = new QWidget;
    QHBoxLayout* horizontal = new QHBoxLayout;
    horizontal->addWidget(button);
    horizontal->addWidget(label);
    container->setLayout(horizontal);

    QLineEdit* tags = new QLineEdit(url);
    QSizePolicy policy3(QSizePolicy::MinimumExpanding, QSizePolicy::Minimum);
    tags->setCursorPosition(0);
    tags->setSizePolicy(policy3);
    tags->setMaximumWidth(450);

    QVBoxLayout* vertical = new QVBoxLayout;
    vertical->addWidget(container);
    vertical->addWidget(tags);

    delegate->setLayout(vertical);

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

