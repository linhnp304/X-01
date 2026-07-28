#pragma once

#include <QPolygonF>
#include <QString>
#include <QVector>

/// Một hình trong shapefile: gồm nhiều "phần" (part),
/// mỗi phần là một dãy điểm với x = kinh độ, y = vĩ độ (theo hệ toạ độ gốc của tệp).
struct ShpShape
{
    int shapeType = 0;
    QVector<QPolygonF> parts;
};

/// Đọc tệp ESRI Shapefile (.shp) trực tiếp bằng Qt, không cần thư viện ngoài.
/// Hỗ trợ Point(1), PolyLine(3), Polygon(5) và các biến thể Z/M (11,13,15,21,23,25).
namespace ShapefileReader {

/// Trả về danh sách hình trong tệp; trả về danh sách rỗng nếu tệp không tồn tại
/// hoặc không phải shapefile hợp lệ.
QVector<ShpShape> read(const QString &path);

} // namespace ShapefileReader
