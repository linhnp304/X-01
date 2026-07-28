#pragma once

#include <QColor>
#include <QString>
#include <QStringList>

class QApplication;

/// Giao diện tối kiểu kỹ thuật dùng chung cho toàn phần mềm.
namespace Theme {

// --- Bảng màu chuẩn của giao diện (không phải màu nền bản đồ) ---
inline const QColor kBackground{ 0x16, 0x18, 0x1A }; ///< nền sâu nhất (khung ngoài)
inline const QColor kPanel{ 0x1D, 0x20, 0x23 };      ///< nền panel điều khiển
inline const QColor kPanelAlt{ 0x23, 0x27, 0x2A };   ///< nền ô nhập, hàng xen kẽ
inline const QColor kBorder{ 0x31, 0x37, 0x3C };     ///< đường viền
inline const QColor kText{ 0xC6, 0xCC, 0xD2 };       ///< chữ thường
inline const QColor kTextDim{ 0x82, 0x8B, 0x93 };    ///< chữ mờ (nhãn phụ, mục khoá)
inline const QColor kAccent{ 0x4F, 0xB0, 0xD8 };     ///< màu nhấn (tiêu đề nhóm, tab đang chọn)

/// Áp dụng bảng màu + stylesheet cho toàn ứng dụng.
void apply(QApplication &app);

/// Bộ chữ ưu tiên, bảo đảm hiển thị đủ dấu tiếng Việt trên cả Windows và Ubuntu.
QStringList fontFamilies();

} // namespace Theme
