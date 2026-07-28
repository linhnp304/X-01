#pragma once

#include <QString>
#include <QStringList>

/// Đọc thuộc tính từ tệp dBASE (.dbf) đi kèm shapefile.
namespace DbfReader {

/// Trả về giá trị (chuỗi) của một cột cho từng bản ghi, theo đúng thứ tự bản ghi
/// trong tệp .dbf — nhờ đó khớp 1-1 với thứ tự hình trong tệp .shp cùng tên.
/// Trả về danh sách rỗng nếu tệp không đọc được, hoặc danh sách toàn chuỗi rỗng
/// nếu không tìm thấy cột.
QStringList readColumn(const QString &path, const QString &columnName);

} // namespace DbfReader
