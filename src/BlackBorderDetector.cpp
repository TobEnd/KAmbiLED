#include "BlackBorderDetector.h"
#include <QDebug>

BlackBorderDetector::BlackBorderDetector(const QJsonObject &config)
    : _enabled(config.value("enable").toBool(true)),
      _threshold(static_cast<uint8_t>(config.value("threshold").toInt(5) * 2.55)), // Convert 0-100 to 0-255
      _unknownFrameCnt(config.value("unknownFrameCnt").toInt(600)),
      _borderFrameCnt(config.value("borderFrameCnt").toInt(50)),
      _maxInconsistentCnt(config.value("maxInconsistentCnt").toInt(10)),
      _blurRemoveCnt(config.value("blurRemoveCnt").toInt(1)),
      _mode(config.value("mode").toString("default")),
      _currentUnknownFrameCnt(0),
      _currentBorderFrameCnt(0),
      _currentInconsistentCnt(0)
{
}

bool BlackBorderDetector::isBlack(const QColor &color) const
{
    return color.red() < _threshold && color.green() < _threshold && color.blue() < _threshold;
}

BlackBorder BlackBorderDetector::process(const QImage &image)
{
    if (!_enabled || image.isNull()) {
        return BlackBorder(); // Return default unknown border if disabled or image is null
    }

    int width = image.width();
    int height = image.height();

    // Apply blurRemoveCnt (simplified: this would ideally involve image processing)
    // For now, we'll just adjust the sampling area slightly if blurRemoveCnt > 0
    int effectiveWidth = width - (2 * _blurRemoveCnt);
    int effectiveHeight = height - (2 * _blurRemoveCnt);
    int offsetX = _blurRemoveCnt;
    int offsetY = _blurRemoveCnt;

    if (effectiveWidth <= 0 || effectiveHeight <= 0) {
        qWarning() << "BlackBorderDetector: Image too small after blur removal adjustment.";
        return BlackBorder();
    }

    int width33 = effectiveWidth / 3;
    int height33 = effectiveHeight / 3;
    int width66 = width33 * 2;
    int height66 = height33 * 2;
    int xCenter = effectiveWidth / 2;
    int yCenter = effectiveHeight / 2;

    int firstNonBlackX = -1;
    int firstNonBlackY = -1;

    // Find the first non-black pixel from the left edge
    for (int x = 0; x < width33; ++x) {
        if (!isBlack(image.pixelColor(offsetX + x, offsetY + height33)) ||
            !isBlack(image.pixelColor(offsetX + x, offsetY + height66)) ||
            !isBlack(image.pixelColor(offsetX + width - 1 - x, offsetY + yCenter))) {
            firstNonBlackX = x;
            break;
        }
    }

    // Find the first non-black pixel from the top edge
    for (int y = 0; y < height33; ++y) {
        if (!isBlack(image.pixelColor(offsetX + width33, offsetY + y)) ||
            !isBlack(image.pixelColor(offsetX + width66, offsetY + y)) ||
            !isBlack(image.pixelColor(offsetX + xCenter, offsetY + height - 1 - y))) {
            firstNonBlackY = y;
            break;
        }
    }

    BlackBorder currentDetectedBorder;
    if (firstNonBlackX != -1 && firstNonBlackY != -1) {
        currentDetectedBorder.unknown = false;
        currentDetectedBorder.verticalSize = firstNonBlackX + _blurRemoveCnt;
        currentDetectedBorder.horizontalSize = firstNonBlackY + _blurRemoveCnt;
    }

    // State management for consistent detection
    if (currentDetectedBorder.unknown) {
        _currentUnknownFrameCnt++;
        _currentBorderFrameCnt = 0;
        _currentInconsistentCnt = 0;
    } else if (_lastDetectedBorder.unknown || 
               currentDetectedBorder.horizontalSize != _lastDetectedBorder.horizontalSize || 
               currentDetectedBorder.verticalSize != _lastDetectedBorder.verticalSize) {
        _currentInconsistentCnt++;
        _currentUnknownFrameCnt = 0;
        _currentBorderFrameCnt = 0;
    } else {
        _currentBorderFrameCnt++;
        _currentUnknownFrameCnt = 0;
        _currentInconsistentCnt = 0;
    }

    if (_currentUnknownFrameCnt >= _unknownFrameCnt || 
        _currentInconsistentCnt >= _maxInconsistentCnt) {
        _lastDetectedBorder = BlackBorder(); // Reset to unknown
        _currentUnknownFrameCnt = 0;
        _currentInconsistentCnt = 0;
    } else if (_currentBorderFrameCnt >= _borderFrameCnt) {
        _lastDetectedBorder = currentDetectedBorder;
    }

    // The 'mode' parameter is not fully implemented here as it requires more complex image analysis
    // and potentially different sampling strategies. For now, the 'default' mode behavior is assumed.

    return _lastDetectedBorder;
}
