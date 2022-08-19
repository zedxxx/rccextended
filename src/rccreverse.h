#ifndef RCCREVERSE_H
#define RCCREVERSE_H

#include <QDir>
#include <QLocale>
#include <QString>
#include <QResource>

#include "resinfo.h"

class RccReverse
{
public:
    RccReverse();
    void run(const QDir &dir);

private:
    ResInfo m_resInfo;

    QString m_qrc;
    QString m_bat;

    QLocale m_currLocale;

    void extractResourses(const QDir &dir, const QString &path);
    void extractFile(const QString &fileName, const QString &outFileName, const QString &lang);

    void qrcWrite(const QString &resFileName, const QString &outFileName, const QString &lang);
    void qrcSave(const QString &rccFileName);

    void scriptWrite(const QString &rccFileName);
    void scriptSave();

    void updateLocale(const QLocale &locale = QLocale::system());
};

#endif // RCCREVERSE_H
