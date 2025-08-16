#ifndef DEBUGCLIENT_H
#define DEBUGCLIENT_H

#include <QObject>
#include <QImage>
#include <QDebug>

class DebugClient : public QObject
{
    Q_OBJECT

public:
    explicit DebugClient(QObject *parent = nullptr);
    ~DebugClient();

    void sendImage(const QImage &image);
    void setLedsColor(const QVector<QColor> &colors) { Q_UNUSED(colors); qDebug() << "DebugClient: setLedsColor called (dummy implementation)."; }

private:
    // Helper to convert RGB to an ASCII character or colored block
    QString rgbToAnsiColor(int r, int g, int b);
};

#endif // DEBUGCLIENT_H
