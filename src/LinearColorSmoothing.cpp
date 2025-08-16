#include "LinearColorSmoothing.h"
#include <QDebug>
#include <QElapsedTimer>

LinearColorSmoothing::LinearColorSmoothing(const QJsonObject &config)
    : _enabled(config.value("enable").toBool(true)),
      _type(config.value("type").toString("linear")),
      _time_ms(config.value("time_ms").toInt(150)),
      _updateFrequency(config.value("updateFrequency").toDouble(25.0)),
      _interpolationRate(config.value("interpolationRate").toDouble(1.0)),
      _decay(config.value("decay").toDouble(1.0)),
      _dithering(config.value("dithering").toBool(true)),
      _updateDelay(config.value("updateDelay").toInt(0))
{
    _timer.start();
}

QVector<QColor> LinearColorSmoothing::smooth(const QVector<QColor>& newColors)
{
    if (!_enabled || newColors.isEmpty()) {
        _previousColors.clear();
        return newColors;
    }

    if (_previousColors.isEmpty() || _previousColors.size() != newColors.size()) {
        _previousColors = newColors;
        return newColors;
    }

    QVector<QColor> smoothedColors(newColors.size());

    // Calculate the smoothing factor based on time and update frequency
    double smoothingFactor = 0.0;
    if (_updateFrequency > 0) {
        double targetTime = 1000.0 / _updateFrequency;
        double elapsedTime = _timer.elapsed();
        _timer.restart();

        if (_type == "linear") {
            smoothingFactor = qMin(1.0, elapsedTime / targetTime);
        } else if (_type == "decay") {
            // Decay smoothing is more complex and requires a different approach
            // For simplicity, we'll use a basic linear interpolation for now
            // A proper decay implementation would involve exponential smoothing
            smoothingFactor = qMin(1.0, elapsedTime / targetTime);
        }
    }

    for (int i = 0; i < newColors.size(); ++i) {
        const QColor& newColor = newColors.at(i);
        const QColor& prevColor = _previousColors.at(i);

        int r = static_cast<int>(newColor.red() * smoothingFactor + prevColor.red() * (1.0 - smoothingFactor));
        int g = static_cast<int>(newColor.green() * smoothingFactor + prevColor.green() * (1.0 - smoothingFactor));
        int b = static_cast<int>(newColor.blue() * smoothingFactor + prevColor.blue() * (1.0 - smoothingFactor));

        smoothedColors[i] = QColor(r, g, b);
    }

    _previousColors = smoothedColors;
    return smoothedColors;
}
