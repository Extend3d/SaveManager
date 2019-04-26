#include "Headers/contentretriever.h"

#include <QScreen>

ContentRetriever::ContentRetriever(QNetworkAccessManager* manager, QDir rootDir)
{

    this->clientID = "Client-ID 8df2e1247517bc4"; //Imgur client ID

    this->manager = manager;
    connect(manager, &QNetworkAccessManager::finished, this, &ContentRetriever::processResponse);


    rootDir.cd("media");
    this->mediaDir = rootDir;

}

void ContentRetriever::retrieve(SavedEntry* entry) {

    QString type = entry->type;
    QString url = entry->url;
    QString idNum = entry->idNum;

    QString requestUrl;
    if (type == "t3") {

        if (url.contains("i.redd.it")) {
            download(url, idNum);
        }
//        else if (url.contains("v.redd.it")) {
//            download(url, idNum);
//        }
        else if (url.contains("imgur")) {

            QString imgurID = url.split("/").last();
            if (imgurID.contains(".")) {
                imgurID = imgurID.split(".").first();
            }
            if (url.contains("/a/")) {
                requestUrl = "https://api.imgur.com/3/album/" + imgurID;
            } else {
                requestUrl = "https://api.imgur.com/3/image/" + imgurID;
            }
            download(requestUrl, idNum);
        }
//        else if (url.contains("youtube") || url.contains("youtu.be")) {

//            extension += "mp4";

//        }
//        else if (url.contains("/comments/")) {
//            extension = "html";
//        }
        else if (url.contains("gfycat")) {

            QString gfycatID = url.split("/").last();
            if (gfycatID.contains(".")) {
                gfycatID = gfycatID.split(".").first();
            }
            requestUrl = "https://api.gfycat.com/v1/gfycats/" + gfycatID;

            download(requestUrl, idNum);

        }
        else if (url.contains(".jpg")) {
            download(url, idNum);
        }
        else if (url.contains(".png")) {
            download(url, idNum);
        } else {

            download(url, idNum);

        }

    }
    else if (type == "t1") {

        download(url, idNum);

    }
}

void ContentRetriever::download(QString url, QString idNum) {

    //HTTP request, "application/x-www-form-urlencoded" is just a default of some kind
    QNetworkRequest request = QNetworkRequest(QUrl(url));
    request.setHeader(QNetworkRequest::ContentTypeHeader, "application/x-www-form-urlencoded");

    //Imgur api is the only download request that needs any additional headers
    if (url.contains("api.imgur.com/3/")) {
        request.setRawHeader("Authorization", clientID.toUtf8());
    }
    QNetworkReply* reply = manager->get(request);
    reply->setProperty("idNum", idNum);
}

void ContentRetriever::processResponse(QNetworkReply* reply)
{

    QByteArray bytes = reply->readAll();
    int statusCode = reply->attribute(QNetworkRequest::HttpStatusCodeAttribute).toInt();

    QString url = reply->url().toString();
    QString idNum = reply->property("idNum").toString();

    if (statusCode == 200) {

        if (url.contains("api.imgur.com/3/image/")) {
            QString response = QString::fromUtf8(bytes.data(), bytes.size());
            QString imageUrl = response.split("\"link\":\"").last().split("\",").first();
            if (imageUrl.endsWith(".gif")) { imageUrl = response.split("\"mp4\":\"").last().split("\",").first(); }
            imageUrl = imageUrl.replace("\\/", "/");
            download(imageUrl, idNum);

        }
        else if (url.contains("api.imgur.com/3/album/")) {
            QString response = QString::fromUtf8(bytes.data(), bytes.size());
            //TODO download imgur albums


        }
        else if (url.contains("api.gfycat.com/v1/gfycats/")) {
            QString response = QString::fromUtf8(bytes.data(), bytes.size());
            QString mp4Url = response.split("\"mp4Url\":\"").last().split("\",").first();
            download(mp4Url, idNum);
        }
        else {

            QMimeDatabase db;
            QMimeType mime = db.mimeTypeForData(bytes);
            QString extension = "." + mime.preferredSuffix();

            QFile file(mediaDir.path() + "/" + idNum + extension);
            if (!file.open(QFile::WriteOnly)) {
                qFatal("Failed to open media file for writing");
            }
            file.write(bytes);
            file.flush();
            file.close();
        }

    }
    else {
        QFile f(mediaDir.path() + "/log.txt");
        if (!f.open(QIODevice::Append)) {
                qDebug() << "Error opening log file";
        }
        QString out = QString("%1 %2 %3 %4\n").arg(QString::number(statusCode), idNum, reply->errorString(), url);
        f.write(out.toUtf8());
        f.flush();
        f.close();
    }


}
