#ifndef WLEDCONFIGCLIENT_H
#define WLEDCONFIGCLIENT_H

#include <QObject>
#include <QNetworkAccessManager>
#include <QNetworkReply>
#include <QJsonDocument>
#include <QJsonObject>
#include <QJsonArray>

class WledConfigClient : public QObject
{
    Q_OBJECT

public:
    explicit WledConfigClient(const QString &host, QObject *parent = nullptr);
    ~WledConfigClient();

    void getInfo();

signals:
    void infoReceived(const QJsonObject &info);
    void error(const QString &message);

private slots:
    void onInfoReplyFinished(QNetworkReply *reply);

private:
    QNetworkAccessManager *_networkAccessManager;
    QString _wledHost;
};

#endif // WLEDCONFIGCLIENT_H