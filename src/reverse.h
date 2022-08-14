#ifndef REVERSE_H
#define REVERSE_H

#include <qstring.h>
#include <qdir.h>

class RCCReverseLib
{
public:
    RCCReverseLib();
    void rccReverse(const QDir& dir);
private:
    QString m_mask;
    QString m_res_path;
    QString m_qrc_path;
    QString m_bat_path;
    QString m_log_path;
    QString m_qrc;
    QString m_bat;
    QString m_rcc;
    QString m_log;
    void recurRccReverse(const QDir& dir, const QString path);
    void qrcWrite(QString qrc, QString fpath);
    void toLog (const QString text);
};

#endif // REVERSE_H
