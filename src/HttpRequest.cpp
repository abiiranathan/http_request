#include "includes/HttpRequest.h"

#include <QByteArray>
#include <QDebug>
#include <QFile>
#include <QImage>
#include <QImageReader>
#include <QJsonArray>
#include <QJsonDocument>
#include <QJsonObject>
#include <QMap>
#include <QNetworkAccessManager>
#include <QNetworkReply>
#include <QNetworkRequest>

// Initialize the auth token static member variable
QString HttpRequest::AuthToken = QString();
QString HttpRequest::BASE_NAME = QString();

void HttpRequest::SetRootCA(QString certPath) {
    QFile file(certPath);

    if (!file.open(QIODevice::ReadOnly)) {
        qDebug() << "Unable to load root certificate" << file.errorString();
        return;
    }

    const QByteArray bytes = file.readAll();
    const QSslCertificate certificate(bytes);

    // Add custom ca to default ssl configuration
    QSslConfiguration configuration = QSslConfiguration::defaultConfiguration();
    auto certs = configuration.caCertificates();
    certs.append(certificate);

    configuration.setCaCertificates(certs);
    QSslConfiguration::setDefaultConfiguration(configuration);
    file.close();
}

void HttpRequest::initRequest(QString url) {
    manager = new QNetworkAccessManager();
    request = QNetworkRequest(QUrl(BASE_NAME + url));

    request.setRawHeader("Content-Type", "application/json");

    if (AuthToken != "") {
        request.setRawHeader("Authorization", QString("Bearer ").append(AuthToken).toLocal8Bit());
    }
}

HttpRequest::HttpRequest(QString url) { initRequest(url); };

HttpRequest::HttpRequest(QString url, QMap<QString, QString> headers) {
    // Call default constructor
    initRequest(url);

    // Set all request headers onto the request
    QMapIterator<QString, QString> it(headers);
    while (it.hasNext()) {
        it.next();
        request.setRawHeader(it.key().toLocal8Bit(), it.value().toLocal8Bit());
    }
};

HttpRequest::~HttpRequest() {
    delete &request;
    delete manager;
};

void HttpRequest::processJSONResponse(QNetworkReply *reply) {
    if (reply->error() == QNetworkReply::NoError) {
        reply->deleteLater();
        QJsonDocument doc = QJsonDocument::fromJson(reply->readAll());
        emit this->jsonReady(doc);
    } else {
        sendError(reply);
    }
}

void HttpRequest::processHtmlResponse(QNetworkReply *reply) {
    if (reply->error() == QNetworkReply::NoError) {
        reply->deleteLater();
        QByteArray data = reply->readAll();
        emit this->htmlReady(data);
    } else {
        sendError(reply);
    }
}

void HttpRequest::processStringResponse(QNetworkReply *reply) {
    if (reply->error() == QNetworkReply::NoError) {
        reply->deleteLater();
        QString text = reply->readAll();
        emit this->deleteComplete(text);
    } else {
        sendError(reply);
    }
}

void HttpRequest::sendError(QNetworkReply *reply) {
    QJsonDocument doc = QJsonDocument::fromJson(reply->readAll());
    QString defaultError = reply->errorString();
    reply->deleteLater();

    if (!doc.isObject()) {
        emit this->onError(defaultError);
        return;
    }

    QJsonObject obj = doc.object();
    if (obj.contains("errors")) {
        // Loop through errors and concatenate them.
        QString errorString = "";
        QJsonArray errors = obj.value("errors").toArray();

        for (int i = 0; i < errors.size(); i++) {
            QJsonObject o = errors[i].toObject();
            errorString.append(o.value("field").toString())
                .append(o.value("msg").toString())
                .append("\n");
        }
        emit this->onError(errorString);
        return;
    }

    if (obj.contains("error")) {
        emit this->onError(obj.value("error").toString());
        return;
    }

    emit this->onError(defaultError);
}

void HttpRequest::getImage() {
    connect(manager, &QNetworkAccessManager::finished, this, [this](QNetworkReply *reply) {
        QImageReader imageReader(reply);
        QImage img = imageReader.read();
        reply->deleteLater();

        if (img.isNull()) {
            emit this->onError(imageReader.errorString());
            return;
        }

        emit this->imageReady(img);
    });

    manager->get(request);
}

void HttpRequest::getFile(QString path) {
    connect(manager, &QNetworkAccessManager::finished, this, [path, this](QNetworkReply *reply) {
        if (reply->error() == QNetworkReply::NoError) {
            QFile *file;
            file = new QFile(path);

            if (file->open(QIODevice::WriteOnly)) {
                file->write(reply->readAll());
                reply->deleteLater();
                file->close();
                delete file;

                emit this->fileReady(path);
            } else {
                QString err = file->errorString();
                delete file;
                reply->deleteLater();
                emit this->onError("Unable to save the file: " + file->errorString());
            }
        } else {
            QString errorString = reply->errorString();
            reply->deleteLater();
            emit this->onError("Error downloading file: " + errorString);
        }
    });

    QNetworkReply *reply = manager->get(request);
    connect(reply, &QNetworkReply::downloadProgress, this,
            [=](qint64 bytesReceived, qint64 bytesTotal) {
                emit this->onProgress(bytesReceived, bytesTotal);
            });
}

void HttpRequest::getJSON() {
    connect(manager, &QNetworkAccessManager::finished, this,
            [this](QNetworkReply *reply) { this->processJSONResponse(reply); });
    manager->get(request);
};

void HttpRequest::getHtml() {
    connect(manager, &QNetworkAccessManager::finished, this,
            [this](QNetworkReply *reply) { this->processHtmlResponse(reply); });
    manager->get(request);
};

void HttpRequest::post(QByteArray data) {
    connect(manager, &QNetworkAccessManager::finished, this,
            [this](QNetworkReply *reply) { this->processJSONResponse(reply); });
    manager->post(request, data);
};

void HttpRequest::put(QByteArray data) {
    connect(manager, &QNetworkAccessManager::finished, this,
            [this](QNetworkReply *reply) { this->processJSONResponse(reply); });
    manager->put(request, data);
};

void HttpRequest::patch(QByteArray data) {
    connect(manager, &QNetworkAccessManager::finished, this,
            [this](QNetworkReply *reply) { this->processJSONResponse(reply); });
    manager->sendCustomRequest(request, "PATCH", data);
};

void HttpRequest::deleteResource() {
    connect(manager, &QNetworkAccessManager::finished, this,
            [this](QNetworkReply *reply) { this->processStringResponse(reply); });
    manager->deleteResource(request);
};
