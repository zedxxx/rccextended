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
    qint32 offset;

    friend QDebug operator<< (QDebug stream, const ResItem &item)
    {
        stream << "ResItem("
               << "country: " << item.country << ", "
               << "language: " << item.language << ", "
               << "flags: " << item.flags << ", "
               << "last-modified: " << item.last_modified << ", "
               << "size: " << item.size << ", "
               << "offset: " << item.offset
               << ")";
        return stream;
    }
};

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

    explicit ResInfo();
    ~ResInfo();

    bool read(const QString &fileName);

    QList<ResItem> getInfo(const QString &itemName) const;
    const QList<QString> getItemNames() const;
    int getItemsCount() const;
    int getFormatVersion() const;
    int getFlags() const;

private:
    void clear();

    bool parseHeader(const QString &fileName);
    bool parseTree(const QString &dir, const int nodeOffset);

    QString getNodeName(qint32 name_offset) const;

private:
    QMultiHash<QString, ResItem> m_info;
    QList<QString> m_orderedItemNames;

    uchar *m_data = nullptr;
    qsizetype m_data_len = 0;

    int m_formatVersion = 0;
    int m_flags = 0;
    const uchar *m_tree = nullptr;
    const uchar *m_names = nullptr;
    const uchar *m_payloads = nullptr;    
};

#endif // RESINFO_H
