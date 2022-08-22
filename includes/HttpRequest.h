#ifndef HTTPREQUEST_H
#define HTTPREQUEST_H

#include <QByteArray>
#include <QDebug>
#include <QFile>
#include <QImage>
#include <QJsonArray>
#include <QJsonDocument>
#include <QJsonObject>
#include <QMap>
#include <QNetworkAccessManager>
#include <QNetworkReply>
#include <QNetworkRequest>

class HttpRequest : public QObject {
    Q_OBJECT

   private:
    QNetworkAccessManager *manager;
    QNetworkRequest request;

    void initRequest(QString url);
    void processJSONResponse(QNetworkReply *reply);
    void processHtmlResponse(QNetworkReply *reply);
    void processStringResponse(QNetworkReply *reply);
    void sendError(QNetworkReply *reply);

   public:
    static QString AuthToken;                 // JWT Token for Authenticating requets
    static void SetRootCA(QString certPath);  // RootCA for self-signed certificates
    static QString BASE_NAME;                 // API base name e.g http://localhost:8080

    HttpRequest(QString url);
    HttpRequest(QString url, QMap<QString, QString> headers);
    ~HttpRequest();

    // GET Requests
    void getJSON();
    void getHtml();
    void getImage();
    void getFile(QString path);

    // POST request
    void post(QByteArray data);

    // PUT request
    void put(QByteArray data);

    // PATCH request performed with CustomRequest
    void patch(QByteArray data);

    // DELETE request
    void deleteResource();

   signals:
    // Updated as the getFile operation is running.
    void onProgress(qint64, qint64);

    // Returns a JSON document.
    void jsonReady(QJsonDocument);

    // Returns a QImage from the network.
    void imageReady(QImage);

    // Returns Html as QByteArray
    void htmlReady(QByteArray);

    // File has been downloaded
    void fileReady(QString);

    // When the delete request completes
    void deleteComplete(QString);

    // Returns error returned from the server
    void onError(QString);
};

#endif /* HTTPREQUEST_H */
