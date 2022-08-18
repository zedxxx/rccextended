#ifndef LOGGER_H
#define LOGGER_H

#include <QDebug>
#include <QString>

class Logger
{
public:
    explicit Logger(const QString &logFileName);
    ~Logger();

    Logger(Logger const&) = delete;
    void operator=(Logger const&) = delete;

private:
    static void messageHandler(QtMsgType type, const QMessageLogContext &context, const QString &msg);
};

#endif // LOGGER_H
