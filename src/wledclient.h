#ifndef WLEDCLIENT_H
#define WLEDCLIENT_H

#include <QObject>
#include <QUdpSocket>
#include <QHostAddress>
#include <QImage>

class WledClient : public QObject
{
    Q_OBJECT

public:
    explicit WledClient(QString host, ushort port, QString colorOrder = "GRB", int offset = 0, bool clockwise = false, QObject *parent = nullptr);
    ~WledClient();

    void sendImage(const QImage &image);
    void setLedsColor(const QVector<QColor> &colors, int timeout = 1);
    void flashLeds(int startIndex, int count, QColor color);

private:
    void appendColor(QByteArray &datagram, const QColor &color);
    QUdpSocket *_udpSocket;
    QHostAddress _wledHost;
    ushort _wledPort;
    QString _colorOrder;
    int _offset;
    bool _clockwise;

signals:
    void error(QString message);
};

#endif // WLEDCLIENT_H
