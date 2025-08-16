#include "wledconfigclient.h"
#include <QDebug>

WledConfigClient::WledConfigClient(const QString &host, QObject *parent) :
    QObject(parent),
    _wledHost(host)
{
    _networkAccessManager = new QNetworkAccessManager(this);
    connect(_networkAccessManager, &QNetworkAccessManager::finished, this, &WledConfigClient::onInfoReplyFinished);
}

WledConfigClient::~WledConfigClient()
{
    qDebug() << "WledConfigClient destroyed.";
}

void WledConfigClient::getInfo()
{
    QUrl url(QString("http://%1/json/info").arg(_wledHost));
    QNetworkRequest request(url);
    _networkAccessManager->get(request);
    qDebug() << "WledConfigClient: Requesting info from" << url.toString();
}

void WledConfigClient::onInfoReplyFinished(QNetworkReply *reply)
{
    if (reply->error() == QNetworkReply::NoError) {
        QByteArray responseData = reply->readAll();
        QJsonDocument jsonDoc = QJsonDocument::fromJson(responseData);
        if (jsonDoc.isObject()) {
            emit infoReceived(jsonDoc.object());
        } else {
            emit error("Invalid JSON response from WLED /json/info");
        }
    } else {
        emit error(QString("Network error: %1").arg(reply->errorString()));
    }
    reply->deleteLater();
}