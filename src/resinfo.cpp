#include <QDebug>
#include <QtEndian>

#include "resinfo.h"

static const QString LOG_CLASS_ID("[ResInfo]");

ResInfo::ResInfo(const QString &fileName)
    : m_fileName(fileName)
{

}

ResInfo::~ResInfo()
{
    clear();
}

const QList<ResItem> ResInfo::getInfo(const QString &itemName)
{
    return m_info.values(itemName);
}

int ResInfo::getFormatVersion()
{
    return m_formatVersion;
}

int ResInfo::getFlags()
{
    return m_flags;
}

void ResInfo::setFileName(const QString &fileName)
{
    m_fileName = fileName;
    clear();
}

void ResInfo::clear()
{
    m_info.clear();

    if (m_data)
        delete [] m_data;

    m_data = nullptr;
    m_data_len = 0;

    m_formatVersion = 0;
    m_flags = 0;
    m_tree = nullptr;
    m_names = nullptr;
    m_payloads = nullptr;
}

bool ResInfo::read(callback_t onItem, void *userData)
{
    clear();

    if (m_fileName.isEmpty()) {
        qInfo() << LOG_CLASS_ID << "File name is empty!";
        return false;
    }

    if (parseHeader(m_fileName))
        return parseTree(onItem, userData, "", 0);

    return false;
}

bool ResInfo::parseHeader(const QString &fileName)
{
    // copy-paste from "qtbase/src/corelib/io/qresource.cpp"

    // bool QDynamicFileResourceRoot::registerSelf(const QString &f)

    if (!m_data) {
        QFile file(fileName);
        bool ok = false;
        if (file.open(QIODevice::ReadOnly)) {
            qint64 fsize = file.size();
            if (fsize <= std::numeric_limits<qsizetype>::max()) {
                m_data_len = file.size();
                m_data = new uchar[m_data_len];
                ok = (m_data_len == file.read((char*)m_data, m_data_len));
            }
        }
        if (!ok) {
            delete [] m_data;
            m_data = nullptr;
            m_data_len = 0;

            qInfo() << LOG_CLASS_ID << "Failed read file:" << fileName;
            return false;
        }
    }

    // bool QDynamicBufferResourceRoot::registerSelf(const uchar *b, qsizetype size)

    // 5 int "pointers"
    if (m_data_len >= 0 && m_data_len < 20) {
        qInfo() << LOG_CLASS_ID << "Invalid file (size too small):" << m_data_len << "bytes";
        return false;
    }

    //setup the data now
    int offset = 0;

    //magic number
    if (m_data[offset+0] != 'q' ||
        m_data[offset+1] != 'r' ||
        m_data[offset+2] != 'e' ||
        m_data[offset+3] != 's')
    {
        qInfo() << LOG_CLASS_ID << "Invalid magic value";
        return false;
    }
    offset += 4;

    const int version = qFromBigEndian<qint32>(m_data + offset);
    offset += 4;

    const int tree_offset = qFromBigEndian<qint32>(m_data + offset);
    offset += 4;

    const int data_offset = qFromBigEndian<qint32>(m_data + offset);
    offset += 4;

    const int name_offset = qFromBigEndian<qint32>(m_data + offset);
    offset += 4;

    quint32 file_flags = 0;
    if (version >= 3) {
        file_flags = qFromBigEndian<qint32>(m_data + offset);
        //offset += 4;
    }

    // Some sanity checking for sizes. This is _not_ a security measure.
    if (m_data_len >= 0 && (tree_offset >= m_data_len || data_offset >= m_data_len || name_offset >= m_data_len)) {
        qInfo() << LOG_CLASS_ID << "Invalid offset value";
        return false;
    }

    if (version >= 0x01 && version <= 0x03) {
        m_formatVersion = version;
        m_flags = file_flags;

        m_tree = m_data + tree_offset;
        m_names = m_data + name_offset;
        m_payloads = m_data + data_offset;

        return true;
    } else {
        qInfo() << LOG_CLASS_ID << "Unknown format version:" << version;
    }

    return false;
}

QString ResInfo::getNodeName(qint32 name_offset)
{
    const quint16 name_length = qFromBigEndian<qint16>(m_names + name_offset);
    name_offset += 2;
    name_offset += 4; //jump past hash

    QString name;
    name.resize(name_length);
    QChar *strData = name.data();
    qFromBigEndian<ushort>(m_names + name_offset, name_length, strData);

    return name;
}

bool ResInfo::parseTree(callback_t onItem, void *userData, const QString &dir, const int nodeOffset)
{
    int offset = nodeOffset;

    qint32 name_offset = qFromBigEndian<qint32>(m_tree + offset);
    offset += 4;

    // first node is "root"
    QString name = (nodeOffset == 0) ? ":" : getNodeName(name_offset);

    const qint16 flags = qFromBigEndian<qint16>(m_tree + offset);
    offset += 2;

    if ( flags & Flags::Directory ) {
        // Directory

        const qint32 child_count = qFromBigEndian<qint32>(m_tree + offset);
        offset += 4;

        //qDebug() << LOG_CLASS_ID << name << "(childs count:" << child_count << ")";

        const qint32 first_child_offset = qFromBigEndian<qint32>(m_tree + offset);
        //offset += 4;

        const int node_size = 14 + (m_formatVersion >= 0x02 ? 8 : 0);

        for (int i = 0; i < child_count; ++i) {
            // recursion
            if ( !parseTree(onItem, userData, dir + name + "/", (first_child_offset + i) * node_size) ) {
                return false;
            }
        }
    } else {
        // Single file
        const qint16 country = qFromBigEndian<qint16>(m_tree + offset);
        offset += 2;

        const qint16 language = qFromBigEndian<qint16>(m_tree + offset);
        offset += 2;

        const qint32 data_offset = qFromBigEndian<qint32>(m_tree + offset);
        offset += 4;

        quint64 last_modified = 0;
        if (m_formatVersion >= 2) {
            last_modified = qFromBigEndian<quint64>(m_tree + offset);
            //offset += 8;
        }

        const qint32 data_size = qFromBigEndian<qint32>(m_payloads + data_offset);

        name = dir + name;

        ResItem item{country, language, flags, last_modified, data_size};
        m_info.insert(name, item);

        if (onItem) {
            onItem(name, item, userData);
        }

        //qDebug() << LOG_CLASS_ID << name << item;
    }

    return true;
}
