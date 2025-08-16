#include "HyperionProcessor.h"
#include <QDebug>
#include <QtMath>

HyperionProcessor::HyperionProcessor(const LedLayout &layout, const QJsonObject &config, QObject *parent)
    : QObject(parent),
      _layout(layout),
      _config(config),
      _borderDetector(config.value("blackborderdetector").toObject()),
      _lastImageSize(0, 0),
      _colorSmoother(config.value("smoothing").toObject()),
      _colorAlgorithm(config.value("colorAlgorithm").toString("mean_sqrt"))
{
}

QVector<QColor> HyperionProcessor::process(const QImage &image)
{
    qDebug() << "HyperionProcessor::process called.";
    if (image.isNull()) {
        return QVector<QColor>();
    }

    // 1. Detect black borders
    BlackBorder currentBorder = _borderDetector.process(image);

    // 2. Check if we need to rebuild the LED map
    if (image.size() != _lastImageSize || 
        currentBorder.horizontalSize != _lastBorder.horizontalSize || 
        currentBorder.verticalSize != _lastBorder.verticalSize) 
    {
        qDebug() << "Change detected. Rebuilding LED map.";
        qDebug() << "New image size:" << image.size();
        qDebug() << "New border - H:" << currentBorder.horizontalSize << "V:" << currentBorder.verticalSize;
        buildLedMap(image.width(), image.height(), currentBorder);
        _lastImageSize = image.size();
        _lastBorder = currentBorder;
    }

    // 3. Calculate the colors
    QVector<QColor> calculatedColors = calculateLedColors(image);

    // 4. Apply smoothing
    QVector<QColor> finalColors = _colorSmoother.smooth(calculatedColors);

    return finalColors;
}

void HyperionProcessor::buildLedMap(int imageWidth, int imageHeight, const BlackBorder &border)
{
    _ledMap.clear();

    int contentWidth = imageWidth - (2 * border.verticalSize);
    int contentHeight = imageHeight - (2 * border.horizontalSize);

    qDebug() << "buildLedMap: imageWidth=" << imageWidth << "imageHeight=" << imageHeight;
    qDebug() << "buildLedMap: border.verticalSize=" << border.verticalSize << "border.horizontalSize=" << border.horizontalSize;
    qDebug() << "buildLedMap: contentWidth=" << contentWidth << "contentHeight=" << contentHeight;

    if (contentWidth <= 0 || contentHeight <= 0) {
        qWarning() << "No content to process after black border removal. Content W:" << contentWidth << "H:" << contentHeight;
        return;
    }

    int totalLeds = _layout.left + _layout.right + _layout.top + _layout.bottom;
    qDebug() << "buildLedMap: totalLeds=" << totalLeds;
    _ledMap.resize(totalLeds);

    // Define sampling thickness (e.g., 5% of the content height/width, minimum 1 pixel)
    int hSamplingThickness = qMax(1, contentHeight / 20); // 5% of content height
    int vSamplingThickness = qMax(1, contentWidth / 20);  // 5% of content width

    int ledIndex = 0;

    // Bottom LEDs
    for (int i = 0; i < _layout.bottom; ++i, ++ledIndex) {
        double hScanStart = double(i) / _layout.bottom;
        double hScanEnd = double(i + 1) / _layout.bottom;
        int xStart = border.verticalSize + int(hScanStart * contentWidth);
        int xEnd = border.verticalSize + int(hScanEnd * contentWidth);
        int yStart = border.horizontalSize + contentHeight - hSamplingThickness; // Sample from bottom region
        int yEnd = border.horizontalSize + contentHeight; // To the very bottom

        qDebug() << "LED" << ledIndex << ": xStart=" << xStart << "xEnd=" << xEnd << "yStart=" << yStart << "yEnd=" << yEnd;

        for (int x = xStart; x < xEnd; ++x) {
            for (int y = yStart; y < yEnd; ++y) {
                _ledMap[ledIndex].append(QPoint(x, y));
            }
        }
    }

    // Right LEDs
    for (int i = 0; i < _layout.right; ++i, ++ledIndex) {
        double vScanStart = double(i) / _layout.right;
        double vScanEnd = double(i + 1) / _layout.right;
        int yStart = border.horizontalSize + int((1.0 - vScanEnd) * contentHeight); // Sample from top of region
        int yEnd = border.horizontalSize + int((1.0 - vScanStart) * contentHeight); // To bottom of region
        int xStart = border.verticalSize + contentWidth - vSamplingThickness; // Sample from rightmost region
        int xEnd = border.verticalSize + contentWidth; // To the very right

        qDebug() << "LED" << ledIndex << ": xStart=" << xStart << "xEnd=" << xEnd << "yStart=" << yStart << "yEnd=" << yEnd;

        for (int x = xStart; x < xEnd; ++x) {
            for (int y = yStart; y < yEnd; ++y) {
                _ledMap[ledIndex].append(QPoint(x, y));
            }
        }
    }

    // Top LEDs
    for (int i = 0; i < _layout.top; ++i, ++ledIndex) {
        double hScanStart = double(i) / _layout.top;
        double hScanEnd = double(i + 1) / _layout.top;
        int xStart = border.verticalSize + int((1.0 - hScanEnd) * contentWidth); // Sample from right of region
        int xEnd = border.verticalSize + int((1.0 - hScanStart) * contentWidth); // To left of region
        int yStart = border.horizontalSize; // Sample from top region
        int yEnd = border.horizontalSize + hSamplingThickness; // To bottom of region

        qDebug() << "LED" << ledIndex << ": xStart=" << xStart << "xEnd=" << xEnd << "yStart=" << yStart << "yEnd=" << yEnd;

        for (int x = xStart; x < xEnd; ++x) {
            for (int y = yStart; y < yEnd; ++y) {
                _ledMap[ledIndex].append(QPoint(x, y));
            }
        }
    }

    // Left LEDs
    for (int i = 0; i < _layout.left; ++i, ++ledIndex) {
        double vScanStart = double(i) / _layout.left;
        double vScanEnd = double(i + 1) / _layout.left;
        int yStart = border.horizontalSize + int(vScanStart * contentHeight); // Sample from top of region
        int yEnd = border.horizontalSize + int(vScanEnd * contentHeight); // To bottom of region
        int xStart = border.verticalSize; // Sample leftmost region
        int xEnd = border.verticalSize + vSamplingThickness; // To right of region

        qDebug() << "LED" << ledIndex << ": xStart=" << xStart << "xEnd=" << xEnd << "yStart=" << yStart << "yEnd=" << yEnd;

        for (int x = xStart; x < xEnd; ++x) {
            for (int y = yStart; y < yEnd; ++y) {
                _ledMap[ledIndex].append(QPoint(x, y));
            }
        }
    }
}

QVector<QColor> HyperionProcessor::calculateLedColors(const QImage &image)
{
    qDebug() << "HyperionProcessor::calculateLedColors called.";
    QVector<QColor> ledColors;
    if (_ledMap.isEmpty()) {
        return ledColors;
    }

    ledColors.reserve(_ledMap.size());

    for (const auto &pixelList : _ledMap) {
        if (_colorAlgorithm == "mean_sqrt") {
            ledColors.append(getMeanSqrtLedColor(image, pixelList));
        } else if (_colorAlgorithm == "mean") {
            ledColors.append(getAverageColor(image, pixelList));
        } else if (_colorAlgorithm == "max") {
            ledColors.append(getMaxColor(image, pixelList));
        } else {
            // Fallback to mean_sqrt if algorithm is unknown
            ledColors.append(getMeanSqrtLedColor(image, pixelList));
        }
    }

    return ledColors;
}

QColor HyperionProcessor::getAverageColor(const QImage &image, const QVector<QPoint> &pixels) const
{
    if (pixels.isEmpty()) {
        return Qt::black;
    }

    quint64 r = 0, g = 0, b = 0;
    for (const QPoint &pt : pixels) {
        QColor color = image.pixelColor(pt);
        r += color.red();
        g += color.green();
        b += color.blue();
    }

    return QColor(r / pixels.size(), g / pixels.size(), b / pixels.size());
}

QColor HyperionProcessor::getMeanSqrtLedColor(const QImage &image, const QVector<QPoint> &pixels) const
{
    if (pixels.isEmpty()) {
        return Qt::black;
    }

    quint64 r_sq = 0, g_sq = 0, b_sq = 0;
    for (const QPoint &pt : pixels) {
        QColor color = image.pixelColor(pt);
        r_sq += color.red() * color.red();
        g_sq += color.green() * color.green();
        b_sq += color.blue() * color.blue();
    }

    return QColor(static_cast<int>(qSqrt(static_cast<double>(r_sq / pixels.size()))),
                  static_cast<int>(qSqrt(static_cast<double>(g_sq / pixels.size()))),
                  static_cast<int>(qSqrt(static_cast<double>(b_sq / pixels.size()))));
}

QColor HyperionProcessor::getMaxColor(const QImage &image, const QVector<QPoint> &pixels) const
{
    if (pixels.isEmpty()) {
        return Qt::black;
    }

    int maxR = 0, maxG = 0, maxB = 0;
    for (const QPoint &pt : pixels) {
        QColor color = image.pixelColor(pt);
        if (color.red() > maxR) maxR = color.red();
        if (color.green() > maxG) maxG = color.green();
        if (color.blue() > maxB) maxB = color.blue();
    }

    return QColor(maxR, maxG, maxB);
}
