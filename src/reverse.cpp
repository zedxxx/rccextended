#include "reverse.h"

#include <QtCore/QResource>
#include <QtCore/QFile>
#include <QtCore/QTextStream>

#include <iostream>

static const char* bat_file_name = "rcc-make"
#ifdef Q_OS_WIN
  ".bat";
#else
  ".sh";
#endif

RCCReverseLib::RCCReverseLib()
{
    m_res_path = QLatin1String("./qresource/res/");
    m_qrc_path = QLatin1String("./qresource/qrc/");
    m_bat_path = QLatin1String("./qresource/rcc/");
    m_log_path = QLatin1String("./qresource/");
    m_mask     = QLatin1String("*.rcc");
    
    m_rcc = QLatin1String("");
    m_log = QLatin1String("");
    m_bat = QLatin1String("");
    
    const char *opt = "--verbose --compress-algo zlib --format-version 1 --binary";
    
    #ifdef Q_OS_WIN
    m_bat = m_bat + "@echo off" + "\r\n\r\n" + "set rcc=rcc.exe" + "\r\n" + "set opt=" + opt + "\r\n\r\n";
    #else
    m_bat = m_bat + "#!/usr/bin/bash" + "\n\n" + "opt=" + opt + "\n\n";
    #endif
}

void RCCReverseLib::rccReverse(const QDir& dir)
{
    QDir tmp(QLatin1String("./"));
    tmp.mkpath(m_qrc_path);
    tmp.mkpath(m_bat_path);    

    QStringList listFiles = dir.entryList(m_mask.split(QLatin1String(" ")), QDir::Files);

    for (const QString& rccFile : listFiles)
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
        QFile file(m_bat_path + bat_file_name);
        if ( file.open(QIODevice::WriteOnly) )
        {
            QTextStream stream(&file);
            stream << m_bat;
            #ifdef Q_OS_WIN
            stream << "\r\n" << "pause" << "\r\n";
            #endif
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

    QStringList listFiles = dir.entryList(QDir::Files);

    for (const QString& resFile : listFiles)
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

    QStringList listDirs = dir.entryList(QDir::Dirs|QDir::NoDotAndDotDot);

    for (const QString& resDir : listDirs)
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
                #ifdef Q_OS_WIN
                m_bat = m_bat + "%rcc% %opt% ./../qrc/" + qrc + ".qrc" + " -o ./../rcc/" + qrc + ".rcc\r\n";
                #else
                m_bat = m_bat + "rcc ${opt} ./../qrc/" + qrc + ".qrc" + " -o ./../rcc/" + qrc + ".rcc\n";
                #endif
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
