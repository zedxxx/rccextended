#include <QDebug>
#include <QResource>
#include <QFile>
#include <QTextStream>

#include "logger.h"
#include "rccreverse.h"

static const char* bat_file_name = "rcc-make"
#ifdef Q_OS_WIN
  ".bat";
#else
  ".sh";
#endif

static const QString rootPath(QLatin1String("./qresource/"));

RccReverse::RccReverse()
{
    m_resPath = rootPath + QLatin1String("res/");
    m_qrcPath = rootPath + QLatin1String("qrc/");
    m_batPath = rootPath + QLatin1String("rcc/");

    m_rcc = QLatin1String("");    
    m_bat = QLatin1String("");
    
    const char *opt = "--verbose --compress-algo zlib --format-version 1 --binary";
    
    #ifdef Q_OS_WIN
    m_bat = m_bat + "@echo off" + "\r\n\r\n" + "set rcc=rcc.exe" + "\r\n" + "set opt=" + opt + "\r\n\r\n";
    #else
    m_bat = m_bat + "#!/usr/bin/bash" + "\n\n" + "opt=" + opt + "\n\n";
    #endif
}

void RccReverse::run(const QDir &dir)
{
    Logger logger(rootPath + QLatin1String("log.txt"));

    QDir tmp(QLatin1String("./"));
    tmp.mkpath(m_qrcPath);
    tmp.mkpath(m_batPath);

    const QStringList listFiles = dir.entryList(QStringList(QLatin1String("*.rcc")), QDir::Files);

    for (const QString &rccFile : listFiles) {
        qInfo() << "Found file:" << rccFile;

        QResource rcc;

        rcc.registerResource(rccFile);

        m_rcc = rccFile;

        qrcWrite("", "");

        recurRccReverse(QDir(":/"), m_resPath + rccFile + "/");

        qrcWrite(rccFile, "");

        rcc.unregisterResource(rccFile);
    }

    if (m_bat != "") {
        QFile file(m_batPath + bat_file_name);
        if ( file.open(QIODevice::WriteOnly) ) {
            QTextStream stream(&file);
            stream << m_bat;
            #ifdef Q_OS_WIN
            stream << "\r\n" << "pause" << "\r\n";
            #endif
        }
        file.close();
    }
}

void RccReverse::recurRccReverse(const QDir &dir, const QString &path)
{
    QDir tmp("./");

    if ( tmp.mkpath(path) )
        qInfo() << "Create folder:" << path;

    dir.addSearchPath(":", dir.path());

    const QStringList listFiles = dir.entryList(QDir::Files);

    for (const QString &resFile : listFiles) {
        qInfo() << "Found resource file:" << dir.absoluteFilePath(resFile);

        QFile file(dir.absoluteFilePath(resFile));

        file.open(QIODevice::ReadWrite);

        if (file.copy(dir.absoluteFilePath(resFile), path + resFile)) {
            qrcWrite("", dir.absoluteFilePath(resFile));

            qInfo() << "Resource restored to file:" << path + resFile;
        }
        file.close();
    }

    const QStringList listDirs = dir.entryList(QDir::Dirs | QDir::NoDotAndDotDot);

    for (const QString &resDir : listDirs) {
        qInfo() << "Found resource folder:" << resDir;
        if (dir.path() != ":/") {
            recurRccReverse(QDir(dir.path() + "/" + resDir), path + resDir + "/");
        } else {
            recurRccReverse(QDir(dir.path() + resDir), path + resDir + "/");
        }
    }
}

void RccReverse::qrcWrite(QString qrc, QString path)
{
    if (path != "") {
        path.replace(":/", "");
        m_qrc = m_qrc + "<file alias=\"" + path + "\">./../res/" + m_rcc + "/" + path + "</file>\n";
    } else {
        if (qrc == "") {
            m_qrc = QLatin1String("<!DOCTYPE RCC><RCC version=\"1.0\">\n"
                                  "<qresource>\n");
        } else {
            m_qrc = m_qrc + QLatin1String("</qresource>\n"
                                          "</RCC>\n");

            qrc.replace(".rcc", "");
            QFile file(m_qrcPath + qrc + ".qrc");
            if ( file.open(QIODevice::WriteOnly) ) {
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
