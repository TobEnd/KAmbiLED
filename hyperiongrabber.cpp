#include "hyperiongrabber.h"
#include <QCoreApplication>
#include <QScreen>
#include <QGuiApplication>
#include <QBuffer>
#include <QImage>

// public

HyperionGrabber::HyperionGrabber(QHash<QString, QString> opts)
{
    QString addr = "192.168.1.177"; // Default WLED IP
    unsigned short port = 4048; // Default DDP port
    unsigned short scale = 8;
    unsigned short frameskip = 0;
    int inactiveTime = 0;

    _scale_m = scale;
    _frameskip_m = frameskip;
    _inactiveTime_m = inactiveTime;

    QHashIterator<QString, QString> i(opts);
    while (i.hasNext()) {
        i.next();
        if ((i.key() == "a" || i.key() == "address") && !(i.value().isNull() && i.value().isEmpty())) {
            addr = i.value();
        } else if ((i.key() == "p" || i.key() == "port") && i.value().toUShort()) {
            port = i.value().toUShort();
        } else if ((i.key() == "s" || i.key() == "scale") && i.value().toUShort()) {
            _scale_m = i.value().toUShort();
        } else if ((i.key() == "f" || i.key() == "frameskip") && i.value().toUShort()) {
            _frameskip_m = i.value().toUShort();
        } else if ((i.key() == "i" || i.key() == "inactive") && i.value().toInt()) {
            _inactiveTime_m = (i.value().toInt() * 1000);
        }
    }

    // _client_p = new WledClient(addr, port, this); // Removed as client is now handled by GrabberConfigurator

    _waylandGrabber_p = new WaylandGrabber(this);
    connect(_waylandGrabber_p, &WaylandGrabber::frameReady, this, &HyperionGrabber::_processFrame);
    _waylandGrabber_p->start();
    
    QScreen *screen = QGuiApplication::primaryScreen();
    if (screen) {
        qDebug() << "Screen size:" << screen->size().width() << "x" << screen->size().height();
    } else {
        qWarning() << "Could not get primary screen size for Wayland grabber. Defaulting to 1920x1080.";
    }
}

HyperionGrabber::~HyperionGrabber()
{
    if (_timer_p) {
        _timer_p->stop();
        delete _timer_p;
    }
    // delete _client_p; // Removed as client is now handled by GrabberConfigurator
}

// private slots

void HyperionGrabber::_processFrame(const QVideoFrame &frame)
{
    if (!frame.isValid()) {
        return;
    }

    _frameCounter_m++;
    if (_frameskip_m > 0 && (_frameCounter_m % (_frameskip_m + 1) != 0)) {
        return;
    }

    QImage image = frame.toImage();
    if (image.isNull()) {
        qWarning() << "Failed to convert QVideoFrame to QImage.";
        return;
    }

    // Calculate target size
    QSize targetSize(image.width() / _scale_m, image.height() / _scale_m);
    if (!targetSize.isValid()) {
        qWarning() << "Invalid target size for scaling.";
        return;
    }

    // Scale the image using Qt's optimized scaling
    QImage scaledImage = image.scaled(targetSize, Qt::IgnoreAspectRatio, Qt::SmoothTransformation);

    // Convert to a format suitable for WLED (RGB888)
    if (scaledImage.format() != QImage::Format_RGB888) {
        scaledImage = scaledImage.convertToFormat(QImage::Format_RGB888);
    }

    // Calculate image difference
    if (!_lastScaledImage_m.isNull() && scaledImage.size() == _lastScaledImage_m.size()) {
        quint64 diff = 0;
        for (int y = 0; y < scaledImage.height(); ++y) {
            QRgb *line = reinterpret_cast<QRgb*>(scaledImage.scanLine(y));
            QRgb *lastLine = reinterpret_cast<QRgb*>(_lastScaledImage_m.scanLine(y));
            for (int x = 0; x < scaledImage.width(); ++x) {
                diff += qAbs(qRed(line[x]) - qRed(lastLine[x]));
                diff += qAbs(qGreen(line[x]) - qGreen(lastLine[x]));
                diff += qAbs(qBlue(line[x]) - qBlue(lastLine[x]));
            }
        }

        if (diff < _changeThreshold_m) {
            // Image has not changed significantly, do not emit
            return;
        }
    }

    _lastScaledImage_m = scaledImage; // Store the current image for next comparison
    emit imageReady(scaledImage);
}