#include <QCommandLineParser>
#include <signal.h>
#include <QFile>
#include <QTextStream>
#include <QDateTime>
#include <QJsonDocument>
#include <QJsonObject>
#include <QCoreApplication>
#include <QTimer>
#include <QApplication> // Added for QApplication
#include <iostream> // Added for std::cout

// Function to load environment variables from .env file
QHash<QString, QString> loadEnvFile(const QString& filePath)
{
    QHash<QString, QString> envVars;
    QFile file(filePath);
    if (!file.open(QIODevice::ReadOnly | QIODevice::Text)) {
        qWarning() << "Could not open .env file:" << filePath;
        return envVars;
    }

    QTextStream in(&file);
    while (!in.atEnd()) {
        QString line = in.readLine().trimmed();
        if (line.isEmpty() || line.startsWith('#')) {
            continue; // Skip empty lines and comments
        }

        int equalsIndex = line.indexOf('=');
        if (equalsIndex > 0) {
            QString key = line.left(equalsIndex).trimmed();
            QString value = line.mid(equalsIndex + 1).trimmed();
            // Remove quotes if present
            if (value.startsWith('"') && value.endsWith('"')) {
                value = value.mid(1, value.length() - 2);
            }
            envVars.insert(key, value);
        }
    }
    file.close();
    return envVars;
}

#include "hyperiongrabber.h"
#include "HyperionProcessor.h"
#include "wledclient.h"
#include "LedColorMapping.h"
#include "LinearColorSmoothing.h"

// Global pointers for cleanup
static HyperionGrabber *grabber = nullptr;
static QApplication *qapp = nullptr;
static HyperionProcessor *processor = nullptr;
static WledClient *wledClient = nullptr;
static QTimer *processTimer = nullptr;
static QImage currentImage; // Store the latest captured image
static bool isDebugEnabled = false; // Global flag for debug output

// Custom message handler for logging to file
static QFile *logFile = nullptr;
void customMessageOutput(QtMsgType type, const QMessageLogContext &context, const QString &msg)
{
    // If debug messages are disabled and this is a debug message, return early
    if (type == QtDebugMsg && !isDebugEnabled) {
        return;
    }

    QByteArray localMsg = msg.toLocal8Bit();
    QString logMessage = QString("%1 %2 %3 %4 %5")
        .arg(QDateTime::currentDateTime().toString("yyyy-MM-dd hh:mm:ss"))
        .arg(context.category)
        .arg(context.function)
        .arg(context.line)
        .arg(localMsg.constData());

    switch (type) {
    case QtDebugMsg:
        logMessage = "Debug: " + logMessage;
        break;
    case QtInfoMsg:
        logMessage = "Info: " + logMessage;
        break;
    case QtWarningMsg:
        logMessage = "Warning: " + logMessage;
        break;
    case QtCriticalMsg:
        logMessage = "Critical: " + logMessage;
        break;
    case QtFatalMsg:
        logMessage = "Fatal: " + logMessage;
        break;
    }

    if (logFile && logFile->isOpen()) {
        QTextStream stream(logFile);
        stream << logMessage << Qt::endl;
        stream.flush();
    }

    if (type == QtCriticalMsg || type == QtFatalMsg || !logFile || !logFile->isOpen()) {
        fprintf(stderr, "%s\n", logMessage.toLocal8Bit().constData());
    }
}

// Signal handler for graceful shutdown
static void quit(int)
{
    if (processTimer) {
        processTimer->stop();
        delete processTimer;
        processTimer = nullptr;
    }
    if (grabber != nullptr) {
        delete grabber;
        grabber = nullptr;
    }
    if (processor != nullptr) {
        delete processor;
        processor = nullptr;
    }
    if (wledClient != nullptr) {
        delete wledClient;
        wledClient = nullptr;
    }
    if (logFile) {
        logFile->close();
        delete logFile;
        logFile = nullptr;
    }
    if (qapp != nullptr) {
        qapp->exit();
    }
}

// Slot to receive and store the latest image from HyperionGrabber
void onImageReady(const QImage &image)
{
    currentImage = image;
}

// Slot to process the current image and send to WLED (triggered by timer)
void processCurrentFrame()
{
    if (currentImage.isNull()) {
        qDebug() << "No image available yet for processing.";
        return;
    }

    // Process the image with HyperionProcessor
    QVector<QColor> ledColors = processor->process(currentImage);

    // Send colors to WLED
    wledClient->setLedsColor(ledColors);

    qDebug() << "Processed frame and sent to WLED.";
}

int main(int argc, char *argv[])
{
    // Initialize logging to file
    logFile = new QFile("output.log");
    if (!logFile->open(QFile::WriteOnly | QFile::Text | QFile::Truncate)) {
        fprintf(stderr, "Warning: Could not open log file output.log for writing.\n");
        delete logFile;
        logFile = nullptr;
    }
    qInstallMessageHandler(customMessageOutput);

    QHash<QString, QString> envVars = loadEnvFile(".env");

    QApplication app(argc, argv); // Use QApplication directly
    qapp = &app; // Assign to global pointer

    signal(SIGINT, quit);
    signal(SIGTERM, quit);

    QApplication::setApplicationName("HyperionGrabber");
    QApplication::setApplicationVersion("0.2");

    QCommandLineParser parser;
    parser.setApplicationDescription("Hyperion X11 Grabber");
    parser.addHelpOption();
    parser.addVersionOption();
    parser.addOption(QCommandLineOption(QStringList() << "d" << "debug", QCoreApplication::translate("main", "Enable verbose debug output.")));
    parser.addOption(QCommandLineOption(QStringList() << "o" << "offset", QCoreApplication::translate("main", "Set the LED offset."), "offset"));
    parser.addOption(QCommandLineOption(QStringList() << "wled-address", QCoreApplication::translate("main", "WLED device IP address or hostname."), "address"));
    parser.addOption(QCommandLineOption(QStringList() << "wled-port", QCoreApplication::translate("main", "WLED UDP Realtime port."), "port"));
    parser.addOption(QCommandLineOption(QStringList() << "scale", QCoreApplication::translate("main", "Grabber scale factor."), "scale"));
    parser.addOption(QCommandLineOption(QStringList() << "frameskip", QCoreApplication::translate("main", "Grabber frameskip."), "frameskip"));
    parser.addOption(QCommandLineOption(QStringList() << "change-threshold", QCoreApplication::translate("main", "Grabber change threshold."), "threshold"));
    parser.addOption(QCommandLineOption(QStringList() << "leds-bottom", QCoreApplication::translate("main", "Number of LEDs on the bottom."), "count"));
    parser.addOption(QCommandLineOption(QStringList() << "leds-right", QCoreApplication::translate("main", "Number of LEDs on the right."), "count"));
    parser.addOption(QCommandLineOption(QStringList() << "leds-top", QCoreApplication::translate("main", "Number of LEDs on the top."), "count"));
    parser.addOption(QCommandLineOption(QStringList() << "leds-left", QCoreApplication::translate("main", "Number of LEDs on the left."), "count"));
    parser.addOption(QCommandLineOption(QStringList() << "bb-enable", QCoreApplication::translate("main", "Enable black border detector."), "enable"));
    parser.addOption(QCommandLineOption(QStringList() << "bb-threshold", QCoreApplication::translate("main", "Black border detector threshold."), "threshold"));
    parser.addOption(QCommandLineOption(QStringList() << "bb-unknown-frame-cnt", QCoreApplication::translate("main", "Black border detector unknown frame count."), "count"));
    parser.addOption(QCommandLineOption(QStringList() << "bb-border-frame-cnt", QCoreApplication::translate("main", "Black border detector border frame count."), "count"));
    parser.addOption(QCommandLineOption(QStringList() << "bb-max-inconsistent-cnt", QCoreApplication::translate("main", "Black border detector max inconsistent count."), "count"));
    parser.addOption(QCommandLineOption(QStringList() << "bb-blur-remove-cnt", QCoreApplication::translate("main", "Black border detector blur remove count."), "count"));
    parser.addOption(QCommandLineOption(QStringList() << "bb-mode", QCoreApplication::translate("main", "Black border detector mode."), "mode"));
    parser.addOption(QCommandLineOption(QStringList() << "smooth-enable", QCoreApplication::translate("main", "Enable smoothing."), "enable"));
    parser.addOption(QCommandLineOption(QStringList() << "smooth-type", QCoreApplication::translate("main", "Smoothing type."), "type"));
    parser.addOption(QCommandLineOption(QStringList() << "smooth-time-ms", QCoreApplication::translate("main", "Smoothing time in milliseconds."), "time"));
    parser.addOption(QCommandLineOption(QStringList() << "smooth-update-frequency", QCoreApplication::translate("main", "Smoothing update frequency."), "frequency"));
    parser.addOption(QCommandLineOption(QStringList() << "smooth-interpolation-rate", QCoreApplication::translate("main", "Smoothing interpolation rate."), "rate"));
    parser.addOption(QCommandLineOption(QStringList() << "smooth-decay", QCoreApplication::translate("main", "Smoothing decay."), "decay"));
    parser.addOption(QCommandLineOption(QStringList() << "smooth-dithering", QCoreApplication::translate("main", "Enable smoothing dithering."), "dithering"));
    parser.addOption(QCommandLineOption(QStringList() << "smooth-update-delay", QCoreApplication::translate("main", "Smoothing update delay."), "delay"));
    parser.addOption(QCommandLineOption(QStringList() << "color-algorithm", QCoreApplication::translate("main", "Color algorithm."), "algorithm"));
    parser.addOption(QCommandLineOption(QStringList() << "clockwise", QCoreApplication::translate("main", "LEDs clockwise."), "clockwise"));
    parser.addOption(QCommandLineOption(QStringList() << "wled-color-order", QCoreApplication::translate("main", "WLED color order."), "order"));

    parser.process(app);

    isDebugEnabled = parser.isSet("debug");
    if (isDebugEnabled) {
        qDebug() << "Debug output enabled.";
    }

    // Read configuration from environment variables and command line
    QString wledAddress = parser.isSet("wled-address") ? parser.value("wled-address") : envVars.value("HYPERION_GRABBER_WLED_ADDRESS");
    if (wledAddress.isEmpty()) {
        qCritical() << "Error: WLED Address not set. Use --wled-address or HYPERION_GRABBER_WLED_ADDRESS environment variable.";
        return 1;
    }

    quint16 wledPort = parser.isSet("wled-port") ? parser.value("wled-port").toUShort() : envVars.value("HYPERION_GRABBER_WLED_PORT").toUShort();
    if (wledPort == 0) wledPort = 21324; // Default WLED UDP Realtime port

    QHash<QString, QString> grabberOpts;
    grabberOpts.insert("scale", parser.isSet("scale") ? parser.value("scale") : (envVars.value("HYPERION_GRABBER_SCALE").isEmpty() ? "8" : envVars.value("HYPERION_GRABBER_SCALE")));
    grabberOpts.insert("frameskip", parser.isSet("frameskip") ? parser.value("frameskip") : (envVars.value("HYPERION_GRABBER_FRAMESKIP").isEmpty() ? "0" : envVars.value("HYPERION_GRABBER_FRAMESKIP")));
    grabberOpts.insert("changeThreshold", parser.isSet("change-threshold") ? parser.value("change-threshold") : (envVars.value("HYPERION_GRABBER_CHANGE_THRESHOLD").isEmpty() ? "100" : envVars.value("HYPERION_GRABBER_CHANGE_THRESHOLD")));

    LedLayout layout;
    layout.bottom = parser.isSet("leds-bottom") ? parser.value("leds-bottom").toInt() : (envVars.value("HYPERION_GRABBER_LEDS_BOTTOM").isEmpty() ? 71 : envVars.value("HYPERION_GRABBER_LEDS_BOTTOM").toInt());
    layout.right = parser.isSet("leds-right") ? parser.value("leds-right").toInt() : (envVars.value("HYPERION_GRABBER_LEDS_RIGHT").isEmpty() ? 20 : envVars.value("HYPERION_GRABBER_LEDS_RIGHT").toInt());
    layout.top = parser.isSet("leds-top") ? parser.value("leds-top").toInt() : (envVars.value("HYPERION_GRABBER_LEDS_TOP").isEmpty() ? 72 : envVars.value("HYPERION_GRABBER_LEDS_TOP").toInt());
    layout.left = parser.isSet("leds-left") ? parser.value("leds-left").toInt() : (envVars.value("HYPERION_GRABBER_LEDS_LEFT").isEmpty() ? 20 : envVars.value("HYPERION_GRABBER_LEDS_LEFT").toInt());

    // Black Border Detector Configuration
    QJsonObject blackBorderDetectorConfig;
    blackBorderDetectorConfig["enable"] = parser.isSet("bb-enable") ? (parser.value("bb-enable").toLower() == "true") : (envVars.value("HYPERION_GRABBER_BB_ENABLE").isEmpty() ? true : (envVars.value("HYPERION_GRABBER_BB_ENABLE").toLower() == "true"));
    blackBorderDetectorConfig["threshold"] = parser.isSet("bb-threshold") ? parser.value("bb-threshold").toInt() : (envVars.value("HYPERION_GRABBER_BB_THRESHOLD").isEmpty() ? 5 : envVars.value("HYPERION_GRABBER_BB_THRESHOLD").toInt());
    blackBorderDetectorConfig["unknownFrameCnt"] = parser.isSet("bb-unknown-frame-cnt") ? parser.value("bb-unknown-frame-cnt").toInt() : (envVars.value("HYPERION_GRABBER_BB_UNKNOWN_FRAME_CNT").isEmpty() ? 600 : envVars.value("HYPERION_GRABBER_BB_UNKNOWN_FRAME_CNT").toInt());
    blackBorderDetectorConfig["borderFrameCnt"] = parser.isSet("bb-border-frame-cnt") ? parser.value("bb-border-frame-cnt").toInt() : (envVars.value("HYPERION_GRABBER_BB_BORDER_FRAME_CNT").isEmpty() ? 50 : envVars.value("HYPERION_GRABBER_BB_BORDER_FRAME_CNT").toInt());
    blackBorderDetectorConfig["maxInconsistentCnt"] = parser.isSet("bb-max-inconsistent-cnt") ? parser.value("bb-max-inconsistent-cnt").toInt() : (envVars.value("HYPERION_GRABBER_BB_MAX_INCONSISTENT_CNT").isEmpty() ? 10 : envVars.value("HYPERION_GRABBER_BB_MAX_INCONSISTENT_CNT").toInt());
    blackBorderDetectorConfig["blurRemoveCnt"] = parser.isSet("bb-blur-remove-cnt") ? parser.value("bb-blur-remove-cnt").toInt() : (envVars.value("HYPERION_GRABBER_BB_BLUR_REMOVE_CNT").isEmpty() ? 1 : envVars.value("HYPERION_GRABBER_BB_BLUR_REMOVE_CNT").toInt());
    blackBorderDetectorConfig["mode"] = parser.isSet("bb-mode") ? parser.value("bb-mode") : (envVars.value("HYPERION_GRABBER_BB_MODE").isEmpty() ? QString("default") : envVars.value("HYPERION_GRABBER_BB_MODE"));

    // Smoothing Configuration
    QJsonObject smoothingConfig;
    smoothingConfig["enable"] = parser.isSet("smooth-enable") ? (parser.value("smooth-enable").toLower() == "true") : (envVars.value("HYPERION_GRABBER_SMOOTH_ENABLE").isEmpty() ? true : (envVars.value("HYPERION_GRABBER_SMOOTH_ENABLE").toLower() == "true"));
    smoothingConfig["type"] = parser.isSet("smooth-type") ? parser.value("smooth-type") : (envVars.value("HYPERION_GRABBER_SMOOTH_TYPE").isEmpty() ? QString("linear") : envVars.value("HYPERION_GRABBER_SMOOTH_TYPE"));
    smoothingConfig["time_ms"] = parser.isSet("smooth-time-ms") ? parser.value("smooth-time-ms").toInt() : (envVars.value("HYPERION_GRABBER_SMOOTH_TIME_MS").isEmpty() ? 150 : envVars.value("HYPERION_GRABBER_SMOOTH_TIME_MS").toInt());
    smoothingConfig["updateFrequency"] = parser.isSet("smooth-update-frequency") ? parser.value("smooth-update-frequency").toDouble() : (envVars.value("HYPERION_GRABBER_SMOOTH_UPDATE_FREQUENCY").isEmpty() ? 25.0 : envVars.value("HYPERION_GRABBER_SMOOTH_UPDATE_FREQUENCY").toDouble());
    smoothingConfig["interpolationRate"] = parser.isSet("smooth-interpolation-rate") ? parser.value("smooth-interpolation-rate").toDouble() : (envVars.value("HYPERION_GRABBER_SMOOTH_INTERPOLATION_RATE").isEmpty() ? 1.0 : envVars.value("HYPERION_GRABBER_SMOOTH_INTERPOLATION_RATE").toDouble());
    smoothingConfig["decay"] = parser.isSet("smooth-decay") ? parser.value("smooth-decay").toDouble() : (envVars.value("HYPERION_GRABBER_SMOOTH_DECAY").isEmpty() ? 1.0 : envVars.value("HYPERION_GRABBER_SMOOTH_DECAY").toDouble());
    smoothingConfig["dithering"] = parser.isSet("smooth-dithering") ? (parser.value("smooth-dithering").toLower() == "true") : (envVars.value("HYPERION_GRABBER_SMOOTH_DITHERING").isEmpty() ? true : (envVars.value("HYPERION_GRABBER_SMOOTH_DITHERING").toLower() == "true"));
    smoothingConfig["updateDelay"] = parser.isSet("smooth-update-delay") ? parser.value("smooth-update-delay").toInt() : (envVars.value("HYPERION_GRABBER_SMOOTH_UPDATE_DELAY").isEmpty() ? 0 : envVars.value("HYPERION_GRABBER_SMOOTH_UPDATE_DELAY").toInt());

    QJsonObject processorConfig;
    processorConfig["blackborderdetector"] = blackBorderDetectorConfig;
    processorConfig["smoothing"] = smoothingConfig;
    processorConfig["colorAlgorithm"] = parser.isSet("color-algorithm") ? parser.value("color-algorithm") : (envVars.value("HYPERION_GRABBER_COLOR_ALGORITHM").isEmpty() ? QString("mean_sqrt") : envVars.value("HYPERION_GRABBER_COLOR_ALGORITHM"));
    processorConfig["offset"] = parser.isSet("offset") ? parser.value("offset").toInt() : (envVars.value("HYPERION_GRABBER_LED_OFFSET").isEmpty() ? 0 : envVars.value("HYPERION_GRABBER_LED_OFFSET").toInt());
    processorConfig["clockwise"] = parser.isSet("clockwise") ? (parser.value("clockwise").toLower() == "true") : (envVars.value("HYPERION_GRABBER_LED_CLOCKWISE").isEmpty() ? false : (envVars.value("HYPERION_GRABBER_LED_CLOCKWISE").toLower() == "true"));

    QString wledColorOrder = parser.isSet("wled-color-order") ? parser.value("wled-color-order") : (envVars.value("HYPERION_GRABBER_WLED_COLOR_ORDER").isEmpty() ? QString("GRB") : envVars.value("HYPERION_GRABBER_WLED_COLOR_ORDER"));

    // Instantiate components
    grabber = new HyperionGrabber(grabberOpts);
    processor = new HyperionProcessor(layout, processorConfig);
    wledClient = new WledClient(wledAddress, wledPort, wledColorOrder, processorConfig["offset"].toInt(), processorConfig["clockwise"].toBool());

    // Connect grabber to image receiver slot
    QObject::connect(grabber, &HyperionGrabber::imageReady, &onImageReady);

    // Setup timer for processing frames
    processTimer = new QTimer();
    processTimer->setInterval(33); // Process every 1 second
    QObject::connect(processTimer, &QTimer::timeout, &processCurrentFrame);
    processTimer->start();

    qInfo() << "HyperionGrabber started. Sending LED colors to WLED device:" << wledAddress << ":" << wledPort;

    // Print all configuration parameters
    std::cout << "--- Configuration ---" << std::endl;
    std::cout << "WLED Address: " << wledAddress.toStdString() << std::endl;
    std::cout << "WLED Port: " << wledPort << std::endl;
    std::cout << "WLED Color Order: " << wledColorOrder.toStdString() << std::endl;
    std::cout << "Grabber Scale: " << grabberOpts.value("scale").toStdString() << std::endl;
    std::cout << "Grabber Frameskip: " << grabberOpts.value("frameskip").toStdString() << std::endl;
    std::cout << "Grabber Change Threshold: " << grabberOpts.value("changeThreshold").toStdString() << std::endl;
    std::cout << "LEDs Bottom: " << layout.bottom << std::endl;
    std::cout << "LEDs Right: " << layout.right << std::endl;
    std::cout << "LEDs Top: " << layout.top << std::endl;
    std::cout << "LEDs Left: " << layout.left << std::endl;
    std::cout << "Black Border Detector Enabled: " << blackBorderDetectorConfig["enable"].toBool() << std::endl;
    std::cout << "Black Border Detector Threshold: " << blackBorderDetectorConfig["threshold"].toInt() << std::endl;
    std::cout << "Black Border Detector Unknown Frame Count: " << blackBorderDetectorConfig["unknownFrameCnt"].toInt() << std::endl;
    std::cout << "Black Border Detector Border Frame Count: " << blackBorderDetectorConfig["borderFrameCnt"].toInt() << std::endl;
    std::cout << "Black Border Detector Max Inconsistent Count: " << blackBorderDetectorConfig["maxInconsistentCnt"].toInt() << std::endl;
    std::cout << "Black Border Detector Blur Remove Count: " << blackBorderDetectorConfig["blurRemoveCnt"].toInt() << std::endl;
    std::cout << "Black Border Detector Mode: " << blackBorderDetectorConfig["mode"].toString().toStdString() << std::endl;
    std::cout << "Smoothing Enabled: " << smoothingConfig["enable"].toBool() << std::endl;
    std::cout << "Smoothing Type: " << smoothingConfig["type"].toString().toStdString() << std::endl;
    std::cout << "Smoothing Time (ms): " << smoothingConfig["time_ms"].toInt() << std::endl;
    std::cout << "Smoothing Update Frequency: " << smoothingConfig["updateFrequency"].toDouble() << std::endl;
    std::cout << "Smoothing Interpolation Rate: " << smoothingConfig["interpolationRate"].toDouble() << std::endl;
    std::cout << "Smoothing Decay: " << smoothingConfig["decay"].toDouble() << std::endl;
    std::cout << "Smoothing Dithering: " << smoothingConfig["dithering"].toBool() << std::endl;
    std::cout << "Smoothing Update Delay: " << smoothingConfig["updateDelay"].toInt() << std::endl;
    std::cout << "Color Algorithm: " << processorConfig["colorAlgorithm"].toString().toStdString() << std::endl;
    std::cout << "LED Offset: " << processorConfig["offset"].toInt() << std::endl;
    std::cout << "LED Clockwise: " << processorConfig["clockwise"].toBool() << std::endl;

    return qapp->exec();
}