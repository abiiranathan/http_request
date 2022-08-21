#HttpRequest

Simple http requests in Qt6 in C++.

HttpRequest in a simple static library and wrapper around **QNetworkAccessManager** that provides a simple interface for API requests in Qt.

This reduces the amount of code you would have to write to do each request in a big Qt application.

## Installation

Install library in your cmake build.

```bash
git clone https://github.com/abiiranathan/http_request

cd http_request
mkdir build && cd build
cmake --build ../
make
sudo make install
```

Add http_request to your Qt CMakeLists.txt(CMake Version 3.5)

```txt
find_library(libhttp_request http_request REQUIRED)

target_link_libraries(TARGET_NAME PRIVATE http_request)
```

### API and Usage

```c++

#include <HttpRequest.h>

#include <QApplication>
#include <QDebug>
#include <QJsonDocument>
#include <QMap>
#include <QtWidgets>

int main(int argc, char **argv) {
    QApplication app(argc, argv);

    QMainWindow *window = new QMainWindow(nullptr);
    QWidget *widget = new QWidget();

    window->setCentralWidget(widget);

    // Set your API Base url
    HttpRequest::BASE_NAME = "https://localhost:8089/api/v1";

    // Set root certificate for your self-signed certs
    HttpRequest::SetRootCA("../certs/rootca.crt");

    // Set JWT AuthToken for Token authentication
    HttpRequest::AuthToken = "your JWT Token";

    // Custom header map
    QMap<QString, QString> headers;

    // Really not neccessary is this is the default content-type
    headers.insert("Accept", "application/json");

    // Initialize a new request to fetch users
    HttpRequest request("/users", headers);

    // GET Request takes in callback functions
    // to receive the result and an error.
    request.get(
        [](QJsonDocument doc) {
            qDebug() << doc;

            // Work with your JSON document
        },
        [](QString error) {
            qDebug() << error;
        });

    window->show();
    return app.exec();
}
```

POST Request

```c++

void handle_login(QJsonObject doc){
  QJsonObject obj = doc.object();

  qDebug() << "Token: " << obj["token"].toString();
  qDebug() << "User: " << obj["user"].toObject();

}

void handle_error(QString error){
  qDebug() << error;
}

QString username = ui->username.text();
QString password = ui->password.text();

QJsonObject obj;
obj["username"] = username;
obj["password"] = password;
QJsonDocument doc(obj);
QByteArray data = doc.toJson();

HttpRequest request("/auth/login")
request.post(data, handle_login, handle_error);

```

Download/Fetch a file

```c++
void onSuccess(){
  qDebug() << "Download complete.";
}

void onError(QString error){
  qDebug() << error;
}

void onProgress(qint64 size, qint64 maxSize){
  count << "Downloaded " << size << " out of " << maxSize ;
}

HttpRequest request("/static/media/video.mp4");
QString destination = QString("/home/myname/Downloads/video.mp4")
request.getFile(
  destination,
  onSuccess,
  onError,
  onProgress);
```

Fetch an image

```c++
void onSuccess(QImage img){
  qDebug() << "Download complete.";
  ui->myLabel->setPixmap(QPixmap::fromImage(img));
}


void onError(QString error){
  qDebug() << error;
}

HttpRequest request("/static/images/logo.png")
request.getImage(onSuccess onError);
```
