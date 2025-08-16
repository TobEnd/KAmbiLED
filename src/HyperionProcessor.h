#ifndef HYPERIONPROCESSOR_H
#define HYPERIONPROCESSOR_H

#include <QObject>
#include <QImage>
#include <QVector>
#include <QColor>
#include <QJsonObject>
#include "LedColorMapping.h" // We can reuse the LedLayout struct
#include "BlackBorderDetector.h"
#include "LinearColorSmoothing.h"

class HyperionProcessor : public QObject
{
    Q_OBJECT

public:
    explicit HyperionProcessor(const LedLayout &layout, const QJsonObject &config, QObject *parent = nullptr);

    QVector<QColor> process(const QImage &image);

private:
    void buildLedMap(int imageWidth, int imageHeight, const BlackBorder &border);
    QVector<QColor> calculateLedColors(const QImage &image);
    QColor getAverageColor(const QImage &image, const QVector<QPoint> &pixels) const;
    QColor getMeanSqrtLedColor(const QImage &image, const QVector<QPoint> &pixels) const;
    QColor getMaxColor(const QImage &image, const QVector<QPoint> &pixels) const;

    LedLayout _layout;
    QJsonObject _config;
    QString _colorAlgorithm;

    BlackBorderDetector _borderDetector;
    BlackBorder _lastBorder;
    QSize _lastImageSize;

    QVector<QVector<QPoint>> _ledMap;

    LinearColorSmoothing _colorSmoother;

public:
    QSize getLastImageSize() const { return _lastImageSize; }
    BlackBorder getLastBorder() const { return _lastBorder; }
    const QVector<QVector<QPoint>>& getLedMap() const { return _ledMap; }
};

#endif // HYPERIONPROCESSOR_H
