#include <QDateTime>
#include <QFile>

#include "logger.h"

static bool isOk_ = false;
static QFile *logFile_ = nullptr;
static QtMessageHandler defaultMessageHandler_ = qInstallMessageHandler(nullptr);

Logger::Logger(const QString &logFileName)
{
    if (isOk_) {
        return;
    }

    logFile_ = new QFile(logFileName);
    isOk_ = logFile_->open(QIODevice::WriteOnly | QIODevice::Text);

    if (isOk_) {
        defaultMessageHandler_ = qInstallMessageHandler(Logger::messageHandler);
    }
}

Logger::~Logger()
{
    if (isOk_) {
        qInstallMessageHandler(defaultMessageHandler_);
        logFile_->close();
        delete logFile_;
        isOk_ = false;
    }
}

void Logger::messageHandler(QtMsgType type, const QMessageLogContext &context, const QString &msg)
{
    if (isOk_) {
        QTextStream out(logFile_);

        //out << QDateTime::currentDateTime().toString("yyyy-MM-dd hh:mm:ss.zzz ");
        out << msg << Qt::endl;
        //out.flush();
    }

    (*defaultMessageHandler_)(type, context, msg);
}
