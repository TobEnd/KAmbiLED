#ifndef LINEARCOLORSMOOTHING_H
#define LINEARCOLORSMOOTHING_H

#include <QVector>
#include <QColor>
#include <QJsonObject>
#include <QElapsedTimer>

class LinearColorSmoothing
{
public:
    explicit LinearColorSmoothing(const QJsonObject &config);

    QVector<QColor> smooth(const QVector<QColor>& newColors);

private:
    bool _enabled;
    QString _type;
    int _time_ms;
    double _updateFrequency;
    double _interpolationRate;
    double _decay;
    bool _dithering;
    int _updateDelay;

    QVector<QColor> _previousColors;
    QElapsedTimer _timer;
};

#endif // LINEARCOLORSMOOTHING_H
