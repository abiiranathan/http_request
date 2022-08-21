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

// HttpRequest is a wrapper around QNetworkAccessManager that eases the processing
// of doing http in Qt application.
//
// It inherits from QObject to be able to use signals and slots.
// Http responses are passed to the caller asyncronous through onSuccess and
// onError callback functions.
//
// Example workflow for login:
// HttpRequest request("BASE_NAME/api/v1/auth/login");
// QJsonObject obj;
// obj["username"] = "username";
// obj["password"] = "password";
// QJsonDocument doc(obj);
// QByteArray data = doc.toJson();
// request.post(
//     data,
//     [](QJsonDocument res) {
//         HttpRequest::AuthToken = res.object().value("token").toString();
//     },
//     [](QString err) {
//         qDebug() << err;
//     });
class HttpRequest : public QObject {
    Q_OBJECT

   private:
    QNetworkAccessManager *manager;
    QNetworkRequest request;

    void processJSONResponse(QNetworkReply *reply, void (*onSuccess)(QJsonDocument), void (*onError)(QString));
    void processHtmlResponse(QNetworkReply *reply, void (*onSuccess)(QByteArray), void (*onError)(QString));
    void processStringResponse(QNetworkReply *reply, void (*onSuccess)(QString), void (*onError)(QString));
    void sendError(QNetworkReply *reply, void (*onError)(QString));
    void initRequest(QString url);

   public:
    static QString AuthToken;                 // JWT Token for Authenticating requets
    static void SetRootCA(QString certPath);  // RootCA for self-signed certificates
    static QString BASE_NAME;                 // API base name e.g http://localhost:8080

    HttpRequest(QString url);
    HttpRequest(QString url, QMap<QString, QString> headers);

    ~HttpRequest();

    // GET JSON requests
    void get(void (*onSuccess)(QJsonDocument), void (*onError)(QString));

    // GET webpage's raw html
    void get(void (*onSuccess)(QByteArray), void (*onError)(QString));

    // Download an image
    void getImage(void (*onSuccess)(QImage), void (*onError)(QString));

    // Download a file
    void getFile(QString path, void (*onSuccess)(), void (*onError)(QString), void (*onDownloadProgess)(qint64, qint64));

    // POST request
    void post(QByteArray data, void (*onSuccess)(QJsonDocument),
              void (*onError)(QString));

    // PUT request
    void put(QByteArray data, void (*onSuccess)(QJsonDocument),
             void (*onError)(QString));

    // PATCH request performed with CustomRequest
    void patch(QByteArray data, void (*onSuccess)(QJsonDocument),
               void (*onError)(QString));

    // DELETE request
    void deleteResource(void (*onSuccess)(QString), void (*onError)(QString));
};

#endif /* HTTPREQUEST_H */
