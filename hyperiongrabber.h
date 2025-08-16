#ifndef HYPERIONGRABBER_H
#define HYPERIONGRABBER_H

#include <QObject>
#include <QTimer>
#include <QImage>
#include <QVideoFrame>

#ifdef DEBUG_MODE
#include "debugclient.h"
#else
#include "wledclient.h"
#endif

#include "WaylandGrabber.h"

class HyperionGrabber : public QObject
{
    Q_OBJECT
public:
    HyperionGrabber(QHash<QString, QString> opts);
    ~HyperionGrabber();

signals:
    void imageReady(const QImage &image);

private:
#ifdef DEBUG_MODE
    DebugClient *_client_p; // Use DebugClient in debug mode
#else
    WledClient *_client_p; // Use WledClient in normal mode
#endif
    QTimer *_timer_p;
    WaylandGrabber *_waylandGrabber_p;

    int _inactiveTime_m = 0;
    unsigned short _scale_m = 8;
    unsigned short _frameskip_m = 0;
    long long _frameCounter_m = 0;
    quint64 _lastImageChecksum_m = 0; // Added for image stability check
    QImage _lastScaledImage_m; // Stores the previously sent image for comparison
    int _changeThreshold_m = 100; // Threshold for image change detection

private slots:
    void _processFrame(const QVideoFrame &frame);
};

#endif // HYPERIONGRABBER_H
