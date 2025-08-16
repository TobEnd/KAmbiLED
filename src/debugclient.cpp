#include "debugclient.h"
#include <QTextStream>

DebugClient::DebugClient(QObject *parent) :
    QObject(parent)
{
    qDebug() << "DebugClient initialized.";
}

DebugClient::~DebugClient()
{
    qDebug() << "DebugClient destroyed.";
}

QString DebugClient::rgbToAnsiColor(int r, int g, int b)
{
    // ANSI escape codes for 256-color terminal
    // Using a simple mapping for now, can be improved for better visual representation
    return QString("\033[48;2;%1;%2;%3m  \033[0m").arg(r).arg(g).arg(b);
}

void DebugClient::sendImage(const QImage &image)
{
    if (image.isNull()) {
        qWarning() << "DebugClient: Cannot display null image.";
        return;
    }

    QTextStream out(stdout);
    out << "\033[H\033[2J"; // Clear screen

    // Iterate through the image pixels and print ASCII representation
    for (int y = 0; y < image.height(); ++y) {
        for (int x = 0; x < image.width(); ++x) {
            QRgb pixel = image.pixel(x, y);
            out << rgbToAnsiColor(qRed(pixel), qGreen(pixel), qBlue(pixel));
        }
        out << Qt::endl;
    }
    out << Qt::endl;
}
