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

HttpRequest::HttpRequest(QString url) {
    initRequest(url);
};

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

HttpRequest::~HttpRequest() = default;

void HttpRequest::processJSONResponse(QNetworkReply *reply, void (*onSuccess)(QJsonDocument), void (*onError)(QString)) {
    if (reply->error() == QNetworkReply::NoError) {
        reply->deleteLater();
        QJsonDocument doc = QJsonDocument::fromJson(reply->readAll());
        onSuccess(doc);
    } else {
        sendError(reply, onError);
    }
}

void HttpRequest::processHtmlResponse(QNetworkReply *reply, void (*onSuccess)(QByteArray), void (*onError)(QString)) {
    if (reply->error() == QNetworkReply::NoError) {
        reply->deleteLater();
        QByteArray data = reply->readAll();
        onSuccess(data);
    } else {
        sendError(reply, onError);
    }
}

void HttpRequest::processStringResponse(QNetworkReply *reply, void (*onSuccess)(QString), void (*onError)(QString)) {
    if (reply->error() == QNetworkReply::NoError) {
        reply->deleteLater();
        QString text = reply->readAll();
        onSuccess(text);
    } else {
        sendError(reply, onError);
    }
}

void HttpRequest::sendError(QNetworkReply *reply, void (*onError)(QString)) {
    QJsonDocument doc = QJsonDocument::fromJson(reply->readAll());
    QString defaultError = reply->errorString();
    reply->deleteLater();

    if (!doc.isObject()) {
        onError(defaultError);
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
        onError(errorString);
        return;
    }

    if (obj.contains("error")) {
        onError(obj.value("error").toString());
        return;
    }

    onError(defaultError);
}

void HttpRequest::getImage(void (*onSuccess)(QImage), void (*onError)(QString)) {
    connect(manager, &QNetworkAccessManager::finished, this,
            [onSuccess, onError](QNetworkReply *reply) {
                QImageReader imageReader(reply);
                QImage img = imageReader.read();
                reply->deleteLater();

                if (img.isNull()) {
                    onError(imageReader.errorString());
                    return;
                }
                return onSuccess(img);
            });

    manager->get(request);
}

void HttpRequest::getFile(QString path, void (*onSuccess)(), void (*onError)(QString), void (*onDownloadProgess)(qint64, qint64)) {
    connect(manager, &QNetworkAccessManager::finished, this,
            [path, onSuccess, onError](QNetworkReply *reply) {
                if (reply->error() == QNetworkReply::NoError) {
                    QFile *file;
                    file = new QFile(path);

                    if (file->open(QIODevice::WriteOnly)) {
                        file->write(reply->readAll());
                        reply->deleteLater();
                        file->close();
                        delete file;
                        onSuccess();
                    } else {
                        QString err = file->errorString();
                        delete file;
                        reply->deleteLater();
                        onError("Unable to save the file: " + file->errorString());
                    }
                } else {
                    QString errorString = reply->errorString();
                    reply->deleteLater();
                    onError("Error downloading file: " + errorString);
                }
            });

    QNetworkReply *reply = manager->get(request);
    if (onDownloadProgess != nullptr) {
        connect(reply, &QNetworkReply::downloadProgress, this, onDownloadProgess);
    }
}

void HttpRequest::get(void (*onSuccess)(QJsonDocument), void (*onError)(QString)) {
    connect(manager, &QNetworkAccessManager::finished, this,
            [this, onSuccess, onError](QNetworkReply *reply) {
                this->processJSONResponse(reply, onSuccess, onError);
            });

    manager->get(request);
};

void HttpRequest::get(void (*onSuccess)(QByteArray), void (*onError)(QString)) {
    connect(manager, &QNetworkAccessManager::finished, this,
            [this, onSuccess, onError](QNetworkReply *reply) {
                this->processHtmlResponse(reply, onSuccess, onError);
            });

    manager->get(request);
};

void HttpRequest::post(QByteArray data, void (*onSuccess)(QJsonDocument), void (*onError)(QString)) {
    connect(manager, &QNetworkAccessManager::finished, this,
            [this, onSuccess, onError](QNetworkReply *reply) {
                this->processJSONResponse(reply, onSuccess, onError);
            });

    manager->post(request, data);
};

void HttpRequest::put(QByteArray data, void (*onSuccess)(QJsonDocument), void (*onError)(QString)) {
    connect(manager, &QNetworkAccessManager::finished, this,
            [this, onSuccess, onError](QNetworkReply *reply) {
                this->processJSONResponse(reply, onSuccess, onError);
            });

    manager->put(request, data);
};

void HttpRequest::patch(QByteArray data, void (*onSuccess)(QJsonDocument), void (*onError)(QString)) {
    connect(manager, &QNetworkAccessManager::finished, this,
            [this, onSuccess, onError](QNetworkReply *reply) {
                this->processJSONResponse(reply, onSuccess, onError);
            });

    manager->sendCustomRequest(request, "PATCH", data);
};

void HttpRequest::deleteResource(void (*onSuccess)(QString), void (*onError)(QString)) {
    connect(manager, &QNetworkAccessManager::finished, this,
            [this, onSuccess, onError](QNetworkReply *reply) {
                this->processStringResponse(reply, onSuccess, onError);
            });

    manager->deleteResource(request);
};
