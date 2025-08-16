#ifndef BLACKBORDERDETECTOR_H
#define BLACKBORDERDETECTOR_H

#include <QImage>
#include <QColor>
#include <QJsonObject>

struct BlackBorder {
    bool unknown = true;
    int horizontalSize = 0;
    int verticalSize = 0;
};

class BlackBorderDetector
{
public:
    explicit BlackBorderDetector(const QJsonObject &config);

    BlackBorder process(const QImage &image);

private:
    bool isBlack(const QColor &color) const;

    bool _enabled;
    uint8_t _threshold;
    int _unknownFrameCnt;
    int _borderFrameCnt;
    int _maxInconsistentCnt;
    int _blurRemoveCnt;
    QString _mode;

    // Internal state for the detector
    int _currentUnknownFrameCnt;
    int _currentBorderFrameCnt;
    int _currentInconsistentCnt;
    BlackBorder _lastDetectedBorder;
};

#endif // BLACKBORDERDETECTOR_H
