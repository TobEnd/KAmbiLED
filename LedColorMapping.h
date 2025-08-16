#ifndef LEDCOLORMAPPING_H
#define LEDCOLORMAPPING_H

#include <QImage>
#include <QVector>
#include <QColor>

struct LedLayout {
    int bottom = 0;
    int right = 0;
    int top = 0;
    int left = 0;
    int offset = 0; // LEDs to skip from the starting corner
    // Future additions: start corner, direction (clockwise/counter-clockwise)
};

class LedColorMapping
{
public:
    static QVector<QColor> map(const QImage &image, const LedLayout &layout);

private:
    static QColor getAverageColor(const QImage &image, int x, int y, int width, int height);
};

#endif // LEDCOLORMAPPING_H
