#ifndef RESINFO_H
#define RESINFO_H

#include <QDebug>
#include <QFile>
#include <QString>

struct ResItem
{
    qint16 country;
    qint16 language;
    qint32 flags;
    quint64 last_modified;
    qint32 size;

    friend QDebug operator<< (QDebug stream, const ResItem &item)
    {
        stream << "ResItem("
               << "country: " << item.country << ", "
               << "language: " << item.language << ", "
               << "flags: " << item.flags << ", "
               << "last-modified: " << item.last_modified << ", "
               << "size: " << item.size
               << ")";
        return stream;
    }
};

typedef void (*callback_t)(const QString &itemName, const ResItem &itemInfo, void *userData);

class ResInfo
{
public:
    enum Flags
    {
        // must match rcc.h
        Compressed = 0x01,
        Directory = 0x02,
        CompressedZstd = 0x04
    };

    explicit ResInfo(const QString &fileName = "");
    ~ResInfo();

    bool read(callback_t onItem = nullptr, void *userData = nullptr);
    const QList<ResItem> getInfo(const QString &itemName);
    int getFormatVersion();
    int getFlags();

    void setFileName(const QString &fileName);
private:
    void clear();

    bool parseHeader(const QString &fileName);
    bool parseTree(callback_t onItem, void *userData, const QString &dir, const int nodeOffset);

    QString getNodeName(qint32 name_offset);

private:
    QString m_fileName;
    QMultiHash<QString, ResItem> m_info;

    uchar *m_data = nullptr;
    qsizetype m_data_len = 0;

    int m_formatVersion = 0;
    int m_flags = 0;
    const uchar *m_tree = nullptr;
    const uchar *m_names = nullptr;
    const uchar *m_payloads = nullptr;    
};

#endif // RESINFO_H
