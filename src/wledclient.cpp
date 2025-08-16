#include "wledclient.h"
#include <QDebug>
#include <QColor>
#include <QByteArray>
#include <QtGlobal>

// DDP Port for WLED
const ushort WLED_DDP_PORT = 4048;
// DNRGB Protocol Type
const quint8 DDP_PROTOCOL_DNRGB = 4;
// Max UDP payload size (approx 508 bytes for safe transmission)
// 4 bytes for DDP header, so 504 bytes for LED data
const int MAX_LED_DATA_PER_PACKET = 504; // 504 bytes / 3 bytes per LED = 168 LEDs

WledClient::WledClient(QString host, ushort port, QString colorOrder, int offset, bool clockwise, QObject *parent) :
    QObject(parent),
    _wledHost(host),
    _wledPort(port),
    _colorOrder(colorOrder),
    _offset(offset),
    _clockwise(clockwise)
{
    _udpSocket = new QUdpSocket(this);
    qDebug() << "WledClient initialized for host:" << _wledHost.toString() << "port:" << _wledPort << "color order:" << _colorOrder << "offset:" << _offset << "clockwise:" << _clockwise;
}

WledClient::~WledClient()
{
    _udpSocket->close();
    qDebug() << "WledClient destroyed.";
}

void WledClient::appendColor(QByteArray &datagram, const QColor &color)
{
    if (_colorOrder == "GRB") {
        datagram.append(color.green());
        datagram.append(color.red());
        datagram.append(color.blue());
    } else if (_colorOrder == "BGR") {
        datagram.append(color.blue());
        datagram.append(color.green());
        datagram.append(color.red());
    } else { // Default to RGB
        datagram.append(color.red());
        datagram.append(color.green());
        datagram.append(color.blue());
    }
}

void WledClient::sendImage(const QImage &image)
{
    if (image.isNull()) {
        qWarning() << "WledClient: Cannot send null image.";
        return;
    }

    // Ensure image is in RGB888 format for direct byte access
    QImage rgbImage = image.convertToFormat(QImage::Format_RGB888);

    int totalLeds = rgbImage.width() * rgbImage.height();
    QVector<QColor> processedColors(totalLeds);

    // Extract colors from image
    for (int i = 0; i < totalLeds; ++i) {
        int x = i % rgbImage.width();
        int y = i / rgbImage.width();
        processedColors[i] = QColor(rgbImage.pixel(x, y));
    }

    // Apply clockwise reversal if needed
    if (_clockwise) {
        QVector<QColor> reversedColors(totalLeds);
        for (int i = 0; i < totalLeds; ++i) {
            reversedColors[i] = processedColors[totalLeds - 1 - i];
        }
        processedColors = reversedColors;
    }

    // Apply offset
    if (_offset != 0) {
        QVector<QColor> shiftedColors(totalLeds);
        for (int i = 0; i < totalLeds; ++i) {
            shiftedColors[i] = processedColors[(i - _offset + totalLeds) % totalLeds];
        }
        processedColors = shiftedColors;
    }

    int ledsPerPacket = MAX_LED_DATA_PER_PACKET / 3; // 3 bytes per LED (RGB)

    qDebug() << "WledClient: sendImage - totalLeds:" << totalLeds << "ledsPerPacket:" << ledsPerPacket;

    for (int i = 0; i < totalLeds; i += ledsPerPacket) {
        QByteArray datagram;
        datagram.reserve(4 + (ledsPerPacket * 3)); // Header + max LED data

        // Byte 0: Protocol type (DNRGB)
        datagram.append(DDP_PROTOCOL_DNRGB);
        // Byte 1: Timeout (5 seconds)
        datagram.append(5);

        // Bytes 2 & 3: Starting LED index (low byte, then high byte)
        quint16 startIndex = i;
        datagram.append(startIndex & 0xFF);         // Low byte
        datagram.append((startIndex >> 8) & 0xFF);  // High byte

        int currentLedsInPacket = qMin(ledsPerPacket, totalLeds - i);

        qDebug() << "WledClient: Sending packet (sendImage) - i:" << i << "startIndex:" << startIndex << "currentLedsInPacket:" << currentLedsInPacket;

        for (int j = 0; j < currentLedsInPacket; ++j) {
            const QColor &color = processedColors.at(i + j);
            appendColor(datagram, color);
        }

        qint64 bytesSent = _udpSocket->writeDatagram(datagram, _wledHost, _wledPort);
        if (bytesSent == -1) {
            qWarning() << "WledClient: Failed to send datagram::" << _udpSocket->errorString();
            emit error(_udpSocket->errorString());
        } else if (bytesSent != datagram.size()) {
            qWarning() << "WledClient: Sent fewer bytes than expected. Expected:" << datagram.size() << "Sent:" << bytesSent;
        }
    }
}

void WledClient::setLedsColor(const QVector<QColor> &colors, int timeout)
{
    if (colors.isEmpty()) {
        return;
    }

    int totalLeds = colors.size();
    QVector<QColor> processedColors(totalLeds);

    // Apply clockwise reversal if needed
    if (_clockwise) {
        for (int i = 0; i < totalLeds; ++i) {
            processedColors[i] = colors[totalLeds - 1 - i];
        }
    } else {
        processedColors = colors;
    }

    // Apply offset
    if (_offset != 0) {
        QVector<QColor> shiftedColors(totalLeds);
        for (int i = 0; i < totalLeds; ++i) {
            shiftedColors[i] = processedColors[(i - _offset + totalLeds) % totalLeds];
        }
        processedColors = shiftedColors;
    }

    int ledsPerPacket = MAX_LED_DATA_PER_PACKET / 3; // 3 bytes per LED (RGB)

    qDebug() << "WledClient: setLedsColor - totalLeds:" << totalLeds << "ledsPerPacket:" << ledsPerPacket;

    for (int i = 0; i < totalLeds; i += ledsPerPacket) {
        QByteArray datagram;
        datagram.reserve(4 + (ledsPerPacket * 3)); // Header + max LED data

        // Byte 0: Protocol type (DNRGB)
        datagram.append(DDP_PROTOCOL_DNRGB);
        // Byte 1: Timeout (in seconds)
        datagram.append(timeout);

        // Bytes 2 & 3: Starting LED index (low byte, then high byte)
        quint16 startIndex = i;
        datagram.append(startIndex & 0xFF);         // Low byte
        datagram.append((startIndex >> 8) & 0xFF);  // High byte

        int currentLedsInPacket = qMin(ledsPerPacket, totalLeds - i);

        qDebug() << "WledClient: Sending packet (setLedsColor) - i:" << i << "startIndex:" << startIndex << "currentLedsInPacket:" << currentLedsInPacket;

        for (int j = 0; j < currentLedsInPacket; ++j) {
            const QColor &color = processedColors.at(i + j);
            appendColor(datagram, color);
        }

        qint64 bytesSent = _udpSocket->writeDatagram(datagram, _wledHost, _wledPort);
        if (bytesSent == -1) {
            qWarning() << "WledClient: Failed to send datagram for setLedsColor:" << _udpSocket->errorString();
            emit error(_udpSocket->errorString());
        } else if (bytesSent != datagram.size()) {
            qWarning() << "WledClient: Sent fewer bytes than expected for setLedsColor. Expected:" << datagram.size() << "Sent:" << bytesSent;
        }
    }
}

void WledClient::flashLeds(int startIndex, int count, QColor color)
{
    if (count <= 0) {
        qWarning() << "WledClient: Count must be positive for flashLeds.";
        return;
    }

    QVector<QColor> colors(count, color);
    int totalLeds = colors.size();
    QVector<QColor> processedColors(totalLeds);

    // Apply clockwise reversal if needed
    if (_clockwise) {
        for (int i = 0; i < totalLeds; ++i) {
            processedColors[i] = colors[totalLeds - 1 - i];
        }
    } else {
        processedColors = colors;
    }

    // Apply offset
    if (_offset != 0) {
        QVector<QColor> shiftedColors(totalLeds);
        for (int i = 0; i < totalLeds; ++i) {
            shiftedColors[i] = processedColors[(i - _offset + totalLeds) % totalLeds];
        }
        processedColors = shiftedColors;
    }

    // Max UDP payload size (approx 508 bytes for safe transmission)
    // 4 bytes for DDP header, so 504 bytes for LED data
    const int MAX_LED_DATA_PER_PACKET = 504; // 504 bytes / 3 bytes per LED (RGB)
    int ledsPerPacket = MAX_LED_DATA_PER_PACKET / 3; // 3 bytes per LED (RGB)

    for (int i = 0; i < count; i += ledsPerPacket) {
        QByteArray datagram;
        datagram.reserve(4 + (ledsPerPacket * 3)); // Header + max LED data

        // Byte 0: Protocol type (DNRGB)
        datagram.append(DDP_PROTOCOL_DNRGB);
        // Byte 1: Timeout (5 seconds)
        datagram.append(5);

        // Bytes 2 & 3: Starting LED index (low byte, then high byte)
        quint16 currentStartIndex = startIndex + i;
        datagram.append(currentStartIndex & 0xFF);         // Low byte
        datagram.append((currentStartIndex >> 8) & 0xFF);  // High byte

        int currentLedsInPacket = qMin(ledsPerPacket, count - i);

        for (int j = 0; j < currentLedsInPacket; ++j) {
            appendColor(datagram, processedColors.at(startIndex + j));
        }

        qDebug() << "WledClient: Sending flashLeds datagram (hex):" << datagram.toHex();
        qDebug() << "WledClient: Datagram size::" << datagram.size();

        qint64 bytesSent = _udpSocket->writeDatagram(datagram, _wledHost, _wledPort);
        if (bytesSent == -1) {
            qWarning() << "WledClient: Failed to send datagram for flashLeds:" << _udpSocket->errorString();
            emit error(_udpSocket->errorString());
        } else if (bytesSent != datagram.size()) {
            qWarning() << "WledClient: Sent fewer bytes than expected for flashLeds. Expected:" << datagram.size() << "Sent:" << bytesSent;
        }
    }
}