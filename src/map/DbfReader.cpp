#include "map/DbfReader.h"

#include <QFile>
#include <QtEndian>

#include <cstring>

QStringList DbfReader::readColumn(const QString &path, const QString &columnName)
{
    QStringList result;

    QFile f(path);
    if (!f.open(QIODevice::ReadOnly))
        return result;
    const QByteArray b = f.readAll();
    f.close();

    if (b.size() < 32)
        return result;

    const auto *u = reinterpret_cast<const uchar *>(b.constData());

    // Header .dbf: byte 4..7 = số bản ghi, byte 8..9 = kích thước header,
    // byte 10..11 = kích thước một bản ghi (đều là little-endian).
    qint32 numRecordsRaw = 0;
    std::memcpy(&numRecordsRaw, b.constData() + 4, 4);
    const int numRecords = qFromLittleEndian(numRecordsRaw);
    const int headerSize = u[8] | (u[9] << 8);
    const int recordSize = u[10] | (u[11] << 8);

    if (numRecords <= 0 || headerSize <= 32 || recordSize <= 0)
        return result;

    // Danh sách mô tả trường: mỗi mô tả dài 32 byte, kết thúc bằng byte 0x0D.
    // Byte đầu của mỗi bản ghi là cờ xoá nên offset trong bản ghi bắt đầu từ 1.
    int fieldOffsetInRecord = 1;
    int targetOffset = -1;
    int targetLength = 0;
    for (int off = 32; off + 32 <= headerSize && u[off] != 0x0D; off += 32) {
        // Tên trường: tối đa 11 byte ASCII, kết thúc bằng byte 0
        int nameLen = 0;
        while (nameLen < 11 && u[off + nameLen] != 0)
            ++nameLen;
        const QString name = QString::fromLatin1(b.constData() + off, nameLen);
        const int len = u[off + 16];

        if (name.compare(columnName, Qt::CaseInsensitive) == 0) {
            targetOffset = fieldOffsetInRecord;
            targetLength = len;
        }
        fieldOffsetInRecord += len;
    }

    result.reserve(numRecords);
    if (targetOffset < 0) {
        // Không có cột yêu cầu: vẫn trả về đúng số phần tử để phía gọi khỏi lệch chỉ số
        for (int r = 0; r < numRecords; ++r)
            result.append(QString());
        return result;
    }

    // Dữ liệu chữ trong các tệp .dbf của bộ MapChuan được lưu bằng UTF-8.
    // Giải mã từng bản ghi độc lập (không dùng bộ giải mã có trạng thái) để một
    // bản ghi hỏng không làm sai lệch các bản ghi sau.
    for (int r = 0; r < numRecords; ++r) {
        const qint64 recOff = static_cast<qint64>(headerSize) + static_cast<qint64>(r) * recordSize;
        if (recOff + recordSize > b.size())
            break;
        const QByteArray raw = b.mid(static_cast<int>(recOff) + targetOffset, targetLength);
        QString value = QString::fromUtf8(raw);
        // Bỏ ký tự đệm (byte 0 và khoảng trắng)
        value.remove(QChar(u'\0'));
        result.append(value.trimmed());
    }

    // Bảo đảm luôn đủ numRecords phần tử
    while (result.size() < numRecords)
        result.append(QString());

    return result;
}
