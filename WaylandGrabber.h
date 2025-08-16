#ifndef WAYLANDGRABBER_H
#define WAYLANDGRABBER_H

#include <QObject>
#include <QScreen>
#include <QMediaCaptureSession>
#include <QScreenCapture>
#include <QVideoSink>
#include <QImage>

class WaylandGrabber : public QObject
{
    Q_OBJECT
public:
    explicit WaylandGrabber(QObject *parent = nullptr);
    void start();

signals:
    void frameReady(const QVideoFrame &frame);

private slots:
    void handleFrame(const QVideoFrame &frame);

private:
    QMediaCaptureSession m_captureSession;
    QScreenCapture m_screenCapture;
    QVideoSink m_videoSink;
};

#endif // WAYLANDGRABBER_H
