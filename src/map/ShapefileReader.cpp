#include "map/ShapefileReader.h"

#include <QFile>
#include <QtEndian>

#include <cstring>

namespace {

/// Đọc số nguyên 32 bit kiểu big-endian tại vị trí off (phần header dùng big-endian).
qint32 readInt32BE(const QByteArray &b, int off)
{
    qint32 v = 0;
    std::memcpy(&v, b.constData() + off, 4);
    return qFromBigEndian(v);
}

/// Đọc số nguyên 32 bit kiểu little-endian (phần nội dung bản ghi dùng little-endian).
qint32 readInt32LE(const QByteArray &b, int off)
{
    qint32 v = 0;
    std::memcpy(&v, b.constData() + off, 4);
    return qFromLittleEndian(v);
}

/// Đọc số thực 64 bit kiểu little-endian.
double readDoubleLE(const QByteArray &b, int off)
{
    quint64 raw = 0;
    std::memcpy(&raw, b.constData() + off, 8);
    raw = qFromLittleEndian(raw);
    double d = 0.0;
    std::memcpy(&d, &raw, 8);
    return d;
}

} // namespace

QVector<ShpShape> ShapefileReader::read(const QString &path)
{
    QVector<ShpShape> shapes;

    QFile f(path);
    if (!f.open(QIODevice::ReadOnly))
        return shapes;
    const QByteArray b = f.readAll();
    f.close();

    // Header của shapefile dài đúng 100 byte
    if (b.size() < 100)
        return shapes;

    // Mã nhận dạng tệp shapefile luôn là 9994
    if (readInt32BE(b, 0) != 9994)
        return shapes;

    // Độ dài tệp ghi trong header tính theo "word" 16 bit -> nhân 2 ra byte
    const int fileLenBytes = readInt32BE(b, 24) * 2;
    const int end = qMin(fileLenBytes, static_cast<int>(b.size()));

    // Duyệt lần lượt từng bản ghi: 8 byte header bản ghi + nội dung
    int i = 100;
    while (i + 12 <= end) {
        const int contentLen = readInt32BE(b, i + 4) * 2;
        const int rec = i + 8;
        if (rec + contentLen > b.size() || contentLen <= 0)
            break;

        const int shapeType = readInt32LE(b, rec);

        ShpShape shape;
        shape.shapeType = shapeType;

        switch (shapeType) {
        case 1:
        case 11:
        case 21: { // Point / PointZ / PointM
            if (rec + 20 > b.size())
                break;
            QPolygonF pt;
            pt << QPointF(readDoubleLE(b, rec + 4), readDoubleLE(b, rec + 12));
            shape.parts.append(pt);
            shapes.append(shape);
            break;
        }
        case 3:
        case 5:
        case 13:
        case 15:
        case 23:
        case 25: { // PolyLine / Polygon (+ biến thể Z, M)
            if (rec + 44 > b.size())
                break;
            const int numParts = readInt32LE(b, rec + 36);
            const int numPoints = readInt32LE(b, rec + 40);
            if (numParts <= 0 || numPoints <= 0)
                break;

            const int partsOff = rec + 44;
            const qint64 pointsOff = static_cast<qint64>(partsOff) + static_cast<qint64>(numParts) * 4;
            if (pointsOff + static_cast<qint64>(numPoints) * 16 > b.size())
                break;

            // Mảng chỉ số điểm bắt đầu của từng part; thêm phần tử cuối = numPoints
            // để tính được số điểm của part cuối cùng.
            QVector<int> partIdx(numParts + 1);
            for (int p = 0; p < numParts; ++p)
                partIdx[p] = readInt32LE(b, partsOff + p * 4);
            partIdx[numParts] = numPoints;

            for (int p = 0; p < numParts; ++p) {
                const int cnt = partIdx[p + 1] - partIdx[p];
                if (cnt <= 0)
                    continue;
                QPolygonF pts;
                pts.reserve(cnt);
                for (int k = 0; k < cnt; ++k) {
                    const qint64 off = pointsOff + static_cast<qint64>(partIdx[p] + k) * 16;
                    pts << QPointF(readDoubleLE(b, static_cast<int>(off)),
                                   readDoubleLE(b, static_cast<int>(off) + 8));
                }
                shape.parts.append(pts);
            }
            shapes.append(shape);
            break;
        }
        default:
            // Null shape (0) và các loại chưa hỗ trợ: bỏ qua
            break;
        }

        i = rec + contentLen;
    }

    return shapes;
}
