#pragma once

#include <QColor>

/// Bảng màu nền bản đồ số theo độ sáng 1..10
/// (1 = tối nhất — xem Screens/map_dark.png, 10 = sáng nhất — xem Screens/map_light.png).
///
/// Mỗi màu được nội suy tuyến tính giữa giá trị ở mức tối nhất và mức sáng nhất,
/// nhờ đó 10 mức chuyển đổi mượt mà chỉ với hai đầu mút được định nghĩa.
struct MapTheme
{
    QColor sea;           // nền biển
    QColor land;          // nền đất liền + đảo
    QColor province;      // viền địa phận tỉnh/thành phố
    QColor nation;        // ranh giới quốc gia + đường biên giới
    QColor coast;         // bờ biển và viền đảo
    QColor label;         // chữ nhãn (tên địa danh, tên sân bay)
    QColor placeDot;      // chấm đánh dấu vị trí địa danh
    QColor airRoute;      // đường bay dân dụng
    QColor airRouteLabel; // tên đường bay
    QColor airport;       // ký hiệu sân bay

    /// Dựng bảng màu ứng với mức sáng brightness (tự kẹp về khoảng 1..10).
    static MapTheme fromBrightness(int brightness);
};
