#include <QDebug>
#include <QResource>
#include <QFile>
#include <QTextStream>

#include "logger.h"
#include "rccreverse.h"

static const QString rootPath("./qresource/");
static const QString resPath(rootPath + "res/");
static const QString qrcPath(rootPath + "qrc/");
static const QString scriptPath(rootPath + "rcc/");

static bool mkpath(const QString &path)
{
    QDir dir("./");
    bool ret = dir.mkpath(path);

    if (!ret)
        qInfo() << "ERROR: Can't create folder:" << path;

    return ret;
}

RccReverse::RccReverse()
{
    m_currLocale = QLocale::system();
}

void RccReverse::run(const QDir &dir)
{
    if (!mkpath(rootPath) || !mkpath(resPath) || !mkpath(qrcPath) || !mkpath(scriptPath) )
        return;

    Logger logger(rootPath + "log.txt");

    const QStringList listFiles = dir.entryList(QStringList("*.rcc"), QDir::Files);

    for (const QString &rccFileName : listFiles) {
        qInfo() << "Processing file:" << rccFileName;

        m_files.clear();

        m_resInfo.setFileName(rccFileName);
        if (!m_resInfo.read()) {
            qInfo() << "WARNING: Only resources for the default/system locale will be extracted";
        }

        updateLocale();

        QResource rcc;
        rcc.registerResource(rccFileName);

        extractResourses(QDir(":/"), resPath + rccFileName + "/");

        rcc.unregisterResource(rccFileName);

        qrcSave(rccFileName);
        scriptWrite(rccFileName);
    }

    scriptSave();
}

void RccReverse::extractResourses(const QDir &dir, const QString &destPath)
{
    if (!mkpath(destPath))
        return;

    dir.addSearchPath(":", dir.path());

    // process files
    const QStringList listFiles = dir.entryList(QDir::Files);

    for (const QString &resFile : listFiles) {

        const QString srcFileName(dir.absoluteFilePath(resFile));
         QString destFileName(destPath + resFile);

        const QList<ResItem> info = m_resInfo.getInfo(srcFileName);

        if (info.isEmpty()) {
            qInfo() << "Found resource file:" << srcFileName;
            updateLocale();
            extractFile(srcFileName, destFileName, "");
        } else {
            for (const ResItem &item : info) {
                QLocale locale(QLocale::Language(item.language),
                               QLocale::Country(item.country));

                bool nameOk = (locale.country() == QLocale::AnyCountry) &&
                              (locale.language() == QLocale::AnyLanguage || locale.language() == QLocale::C);

                QString dest;
                QString lang("");

                if (nameOk) {
                    qInfo() << "Found resource file:" << srcFileName;
                    dest = destFileName;
                } else {
                    qInfo() << "Found resource file:" << srcFileName << locale;
                    lang = locale.bcp47Name();

                    QString path(destPath + lang + "/");
                    if (!mkpath(path)) {
                        continue;
                    }
                    dest = path + resFile;
                }

                updateLocale(locale);
                extractFile(srcFileName, dest, lang);
            }
        }
    }

    // process subfolders (recursion)
    const QStringList listDirs = dir.entryList(QDir::Dirs | QDir::NoDotAndDotDot);

    for (const QString &resDir : listDirs) {
        // qInfo() << "Found resource folder:" << resDir;
        QDir subDir = dir.path() + (dir.path() == ":/" ? "" : "/") + resDir;
        extractResourses(subDir, destPath + resDir + "/");
    }
}

void RccReverse::extractFile(const QString &fileName, const QString &outFileName, const QString &lang)
{
    const QString uniqueFileName = getUniqueFileName(outFileName);

    qrcWrite(fileName, uniqueFileName, lang);

    if (QFile::exists(uniqueFileName)) {
        qInfo() << "File already exists:" << uniqueFileName;
        return;
    }

    if (QFile::copy(fileName, uniqueFileName)) {
        qInfo() << "File extracted to:" << uniqueFileName;
    } else {
        qInfo() << "ERROR: Can't save file to:" << uniqueFileName;
    }
}

const QString RccReverse::getUniqueFileName(const QString &fileName)
{
    #ifdef Q_OS_WIN
    QString storedName(fileName.toLower());
    #else
    QString storedName(fileName);
    #endif

    if (!m_files.contains(storedName)) {
        m_files.insert(storedName, 1);

        return fileName;
    }

    qInfo() << "WARNING: File name collision detected:" << fileName;

    int count = m_files.value(storedName);

    QFileInfo finfo(fileName);

    QString newName = finfo.path() + "/" +
                      finfo.baseName() + "_(" + QString::number(count) + ")" + "." +
                      finfo.completeSuffix();

    #ifdef Q_OS_WIN
    QString newStoredName(newName.toLower());
    #else
    QString newStoredName(newName);
    #endif

    if (!m_files.contains(newStoredName)) {
        m_files.insert(storedName, count+1);
        m_files.insert(newStoredName, 1);

        qInfo() << "INFO: File name collision resolved:" << newName;

        return newName;
    }

    // recursion
    return getUniqueFileName(newName);
}

void RccReverse::qrcWrite(const QString &resFileName, const QString &outFileName, const QString &lang)
{
    QString aliasName(resFileName);
    aliasName.replace(":/", "");

    QString fileName(outFileName);
    fileName.replace(rootPath, "./../");

    const QString alias = "<file alias=\"" + aliasName + "\">" + fileName + "</file>\n";

    if (!lang.isEmpty()) {
        m_qrc = m_qrc +
                "<qresource lang=\"" + lang + "\">" + "\n" +
                "    " + alias +
                "</qresource>" + "\n";
    } else {
        m_qrc = m_qrc + alias;
    }
}

void RccReverse::qrcSave(const QString &rccFileName)
{
    QString fileName(rccFileName);
    fileName.replace(".rcc", "");

    QFile file(qrcPath + fileName + ".qrc");
    if ( file.open(QIODevice::WriteOnly) ) {
        QTextStream stream(&file);

        stream << "<!DOCTYPE RCC><RCC version=\"1.0\">\n"
               << "<qresource>\n"
               <<  m_qrc
               << "</qresource>\n"
               << "</RCC>\n";

        file.close();
    }
}

void RccReverse::scriptWrite(const QString &rccFileName)
{
    QString fileName(rccFileName);
    fileName.replace(".rcc", "");

    #ifdef Q_OS_WIN
    m_bat = m_bat + "%rcc% %opt% ./../qrc/" + fileName + ".qrc" + " -o ./../rcc/" + fileName + ".rcc\r\n";
    #else
    m_bat = m_bat + "rcc ${opt} ./../qrc/" + fileName + ".qrc" + " -o ./../rcc/" + fileName + ".rcc\n";
    #endif
}

void RccReverse::scriptSave()
{
    if (m_bat.isEmpty())
        return;

    #ifdef Q_OS_WIN
      auto ext = ".bat";
    #else
      auto ext = ".sh";
    #endif

    QFile file(scriptPath + "rcc-make" + ext);
    if (file.open(QIODevice::WriteOnly)) {
        QTextStream stream(&file);

        auto opt = "--verbose --compress-algo zlib --format-version 1 --binary";

        #ifdef Q_OS_WIN
        stream << "@echo off" << "\r\n\r\n"
               << "set rcc=rcc.exe" << "\r\n"
               << "set opt=" << opt << "\r\n\r\n";
        #else
        stream << "#!/usr/bin/bash" << "\n\n"
               << "opt=" << opt << "\n\n";
        #endif

        stream << m_bat;

        #ifdef Q_OS_WIN
        stream << "\r\n" << "pause" << "\r\n";
        #endif

        file.close();
    }
}

void RccReverse::updateLocale(const QLocale &locale)
{
    if (m_currLocale != locale) {
        QLocale::setDefault(locale);
        m_currLocale = locale;
    }
}
