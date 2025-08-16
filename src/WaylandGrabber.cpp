#include "WaylandGrabber.h"
#include <QVideoFrame>

WaylandGrabber::WaylandGrabber(QObject *parent)
    : QObject(parent)
{
    m_captureSession.setScreenCapture(&m_screenCapture);
    m_captureSession.setVideoSink(&m_videoSink);

    connect(&m_videoSink, &QVideoSink::videoFrameChanged, this, &WaylandGrabber::handleFrame);
}

void WaylandGrabber::start()
{
    m_screenCapture.start();
}

void WaylandGrabber::handleFrame(const QVideoFrame &frame)
{
    if (frame.isValid())
    {
        emit frameReady(frame);
    }
}