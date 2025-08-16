#include "LedColorMapping.h"
#include <QtMath>

QColor LedColorMapping::getAverageColor(const QImage &image, int x, int y, int width, int height)
{
    if (width <= 0 || height <= 0) {
        return QColor();
    }

    quint64 totalRed = 0;
    quint64 totalGreen = 0;
    quint64 totalBlue = 0;
    int count = 0;

    for (int i = x; i < x + width; ++i) {
        for (int j = y; j < y + height; ++j) {
            if (image.valid(i, j)) {
                totalRed += image.pixelColor(i, j).red();
                totalGreen += image.pixelColor(i, j).green();
                totalBlue += image.pixelColor(i, j).blue();
                count++;
            }
        }
    }

    if (count == 0) {
        return QColor();
    }

    return QColor(totalRed / count, totalGreen / count, totalBlue / count);
}

QVector<QColor> LedColorMapping::map(const QImage &image, const LedLayout &layout)
{
    QVector<QColor> ledColors;
    if (image.isNull() || (layout.bottom + layout.right + layout.top + layout.left) == 0) {
        return ledColors;
    }

    int imgWidth = image.width();
    int imgHeight = image.height();

    // Define the thickness of the border to sample from, e.g., 15% of the smaller dimension
    const float borderPercentage = 0.15;
    int hBorder = qMax(1, int(imgHeight * borderPercentage));
    int vBorder = qMax(1, int(imgWidth * borderPercentage));

    // Reserve space for all LEDs
    ledColors.reserve(layout.bottom + layout.right + layout.top + layout.left);

    // For now, we assume a bottom-left start, going clockwise.
    // This will need to be configurable later.

    // 1. Bottom LEDs (running left to right)
    for (int i = 0; i < layout.bottom; ++i) {
        int regionWidth = imgWidth / layout.bottom;
        int regionX = i * regionWidth;
        ledColors.append(getAverageColor(image, regionX, imgHeight - hBorder, regionWidth, hBorder));
    }

    // 2. Right LEDs (running bottom to top)
    for (int i = 0; i < layout.right; ++i) {
        int regionHeight = imgHeight / layout.right;
        int regionY = imgHeight - (i + 1) * regionHeight; // Start from the bottom
        ledColors.append(getAverageColor(image, imgWidth - vBorder, regionY, vBorder, regionHeight));
    }

    // 3. Top LEDs (running right to left)
    for (int i = 0; i < layout.top; ++i) {
        int regionWidth = imgWidth / layout.top;
        int regionX = imgWidth - (i + 1) * regionWidth; // Start from the right
        ledColors.append(getAverageColor(image, regionX, 0, regionWidth, hBorder));
    }

    // 4. Left LEDs (running top to bottom)
    for (int i = 0; i < layout.left; ++i) {
        int regionHeight = imgHeight / layout.left;
        int regionY = i * regionHeight;
        ledColors.append(getAverageColor(image, 0, regionY, vBorder, regionHeight));
    }

    // Handle offset if necessary (roll the vector)
    if (layout.offset > 0 && !ledColors.isEmpty()) {
        int totalLeds = ledColors.size();
        int offset = layout.offset % totalLeds;
        QVector<QColor> temp = ledColors;
        for(int i=0; i<totalLeds; ++i) {
            ledColors[i] = temp[(i + offset) % totalLeds];
        }
    }

    return ledColors;
}
