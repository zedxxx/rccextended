#include "reverse.h"
#include <QtCore/QResource>
#include <QtCore/QFile>
#include <QtCore/QTextStream>
#include <iostream>

RCCReverseLib::RCCReverseLib()
{
    m_res_path = "./qresource/res/";
    m_qrc_path = "./qresource/qrc/";
    m_bat_path = "./qresource/rcc/";
    m_log_path = "./qresource/";
    m_mask     = "*.rcc";

    m_bat = "";
    m_rcc = "";
    m_log = "";

}

void RCCReverseLib::rccReverse(const QDir& dir)
{
    QDir tmp("./");
    tmp.mkpath(m_qrc_path);
    tmp.mkpath(m_bat_path);    

    QStringList listFiles = dir.entryList(m_mask.split(" "), QDir::Files);

    foreach (QString rccFile, listFiles)
    {
        toLog("Found file: " + rccFile + "\n");

        QResource rcc;

        rcc.registerResource(rccFile);

        m_rcc = rccFile;

        qrcWrite("", "");

        recurRccReverse(QDir(":/"), m_res_path + rccFile + "/");

        qrcWrite(rccFile, "");

        rcc.unregisterResource(rccFile);
    }

    if (m_bat != "")
    {
        QFile file(m_bat_path + "make.bat");
        if ( file.open(QIODevice::WriteOnly) )
        {
            QTextStream stream(&file);
            stream << m_bat;
        }
        file.close();
    }

    if (m_log != "")
    {
        QFile file(m_log_path + "rcc.log");
        if ( file.open(QIODevice::WriteOnly) )
        {
            QTextStream stream(&file);
            stream << m_log;
        }
        file.close();
    }
}

void RCCReverseLib::recurRccReverse(const QDir& dir, const QString path)
{
    QDir tmp("./");

    if ( tmp.mkpath(path) )
        toLog("Create folder: " + path + "\n");

    dir.addSearchPath(":", dir.path());

    // парсим файлы
    QStringList listFiles = dir.entryList(QDir::Files);

    foreach (QString resFile, listFiles)
    {
        toLog("Found resource file: " + dir.absoluteFilePath(resFile) + "\n");

        QFile file(dir.absoluteFilePath(resFile));

        file.open(QIODevice::ReadWrite);

        if (file.copy(dir.absoluteFilePath(resFile), path + resFile))
        {
            qrcWrite("", dir.absoluteFilePath(resFile));

            toLog("Resource restored to file: " + path + resFile + "\n");
        }
        file.close();
    }

    // парсим папки
    QStringList listDirs = dir.entryList(QDir::Dirs|QDir::NoDotAndDotDot);

    foreach (QString resDir, listDirs)
    {
        toLog("Found resource folder: " + resDir + "\n");
        if (dir.path() != ":/") {
            recurRccReverse(QDir(dir.path() + "/" + resDir), path + resDir + "/");
        }
        else
            recurRccReverse(QDir(dir.path() + resDir), path + resDir + "/");
    }
}

void RCCReverseLib::qrcWrite(QString qrc, QString fpath)
{
    if (fpath != "")
    {
        fpath.replace(":/", "");
        m_qrc = m_qrc + "<file alias=\"" + fpath + "\">./../res/" + m_rcc + "/" + fpath + "</file>\n";
    }
    else
    {
        if (qrc == "")
        {
            m_qrc = QLatin1String("<!DOCTYPE RCC><RCC version=\"1.0\">\n"
                                  "<qresource>\n");
        }
        else
        {
            m_qrc = m_qrc + QLatin1String("</qresource>\n"
                                          "</RCC>\n");

            qrc.replace(".rcc", "");
            QFile file(m_qrc_path + qrc + ".qrc");
            if ( file.open(QIODevice::WriteOnly) )
            {
                QTextStream stream(&file);
                stream << m_qrc;

                m_bat = m_bat + "rcc.exe -binary ./../qrc/" + qrc + ".qrc" + " -o ./../rcc/" + qrc + ".rcc\n";

            }
            file.close();
        }
    }
}

void RCCReverseLib::toLog (const QString text)
{
    std::cout << text.toLocal8Bit().data();

    m_log = m_log + text;
}
