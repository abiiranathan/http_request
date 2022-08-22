#HttpRequest

Simple http requests in Qt6 in C++.

HttpRequest in a simple static library and wrapper around **QNetworkAccessManager** that provides a simple interface for API requests in Qt.

## Installation

Install library in your cmake build.

```bash
git clone https://github.com/abiiranathan/http_request.git

cd http_request
mkdir -p build && cd build
cmake --configure ../
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

    // Initialize global static member variables for the library.
    HttpRequest::BASE_NAME = "https://localhost:8089/api/v1";

    // Set root certificate for your self-signed certs
    HttpRequest::SetRootCA("../certs/rootca.crt");

    // Set JWT AuthToken for Token authentication
    HttpRequest::AuthToken = ""; // Update this after login


    // Custom header map
    QMap<QString, QString> headers;

    // Really not neccessary is this is the default content-type
    headers.insert("Accept", "application/json");

    // Initialize a new request to fetch users
    HttpRequest *request = new HttpRequest("/users", headers)

    return app.exec();
}
```

### SIGNALS

Data is returned through use of signals.
A number of signals are defined by HttpRequest.

- jsonReady: This is used when you expect JSON data.
- htmlReady: This is used when fetching raw html.
- imageReady: Used to fetch image resources
- fileReady: Used when downloading files.
- onProgress: Sends download progress when getFile is called. Used together with fileReady signal.
- deleteComplete: Used after delete request.
- onError: Used to report the error from the request as a string.

Example usage:

```c++
HttpRequest request("/users", headers);
HttpRequest *request = new HttpRequest("/users")
connect(request, &HttpRequest::jsonReady, this, [this](QJsonDocument doc){
  this->showUsers(doc);
})

request.getJson();

---

HttpRequest *request = new HttpRequest("/static/media/book.pdf")
connect(request, &HttpRequest::onProgress, this, [this](qint64 bytesRec, qint64 totalBytes){
  this->updateProgresBar(bytesRec, totalBytes);
})

connect(request, &HttpRequest::fileReady, this, [this](QString dest){
  qDebug() << "File save in: " << dest;
})

request.getFile("/home/username/Downloads/book.pdf");

---

#Login
HttpRequest *request = new HttpRequest("/auth/login")
QString username = ui->lineEdit->text();
QString password = ui->lineEdit_2->text();

QJsonObject obj;
obj.insert("username", username);
obj.insert("password", password);

QJsonDocument doc(obj);
QByteArray data = doc.toJson();

HttpRequest *request = new HttpRequest("/auth/login");
connect(request, &HttpRequest::jsonReady, this, [this](QJsonDocument doc){
    this->onLoginSuccess(doc);
});

connect(request, &HttpRequest::onError, this, [this](QString error){
    this->onLoginError(error);
});
request->post(data);

--

#Fetch plain html

HttpRequest *request = new HttpRequest("/index.html");
connect(request, &HttpRequest::htmlReady, this, [this](QByteArray html){
    qDebug <<  html;
});

connect(request, &HttpRequest::onError, this, [this](QString error){
    qDebug() << "error fetching page: " << error;
});
request->getHtml(data);


---
# Delete a resource
connect(request, &HttpRequest::deleteComplete, this, [this](QString msg){
    qDebug <<  msg;
});

request.deleteResource();
The success slot receives a QString.
```
