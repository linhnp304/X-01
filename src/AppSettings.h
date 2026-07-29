#pragma once

#include <QColor>
#include <QString>

/// Chế độ vẽ vòng tròn cự ly.
/// Mỗi chế độ gồm một bước "đậm" và (tuỳ chọn) một bước "mảnh" nhỏ hơn.
enum class RangeRingMode {
    Off,    ///< Tắt — không vẽ vòng tròn nào
    R5km,   ///< đậm mỗi 5 km
    R1km,   ///< đậm mỗi 5 km, mảnh mỗi 1 km
    R05km,  ///< đậm mỗi 1 km, mảnh mỗi 0,5 km
    R01km   ///< đậm mỗi 0,5 km, mảnh mỗi 0,1 km
};

/// Chế độ vẽ đường chia độ (đường thẳng từ tâm đài ra vòng cự ly tối đa).
enum class BearingLineMode {
    Off,  ///< Tắt
    D30,  ///< đậm mỗi 30 độ
    D10,  ///< đậm mỗi 30 độ, mảnh mỗi 10 độ
    D5    ///< đậm mỗi 10 độ, mảnh mỗi 5 độ
};

/// Toàn bộ cài đặt của phần mềm, đọc/ghi ra tệp .json nằm cùng thư mục file chạy.
/// Mọi thay đổi của người dùng đều được ghi lại ngay để làm mặc định cho lần chạy sau.
class AppSettings
{
public:
    // ---- Nền bản đồ số ----
    int mapBrightness = 1;      ///< độ sáng bản đồ, 1 (tối nhất) .. 10 (sáng nhất)
    bool showAirRoutes = false; ///< hiện lớp đường bay dân dụng
    bool showAirports = false;  ///< hiện lớp sân bay
    bool showRivers = true;     ///< hiện lớp sông ngòi
    bool showPlaceNames = true; ///< hiện lớp tên địa danh
    bool showProvinces = true;  ///< hiện lớp địa phận tỉnh/thành phố

    // ---- Tâm đài ----
    double radarLat = 21.028500;  ///< vĩ độ tâm đài (độ)
    double radarLng = 105.854200; ///< kinh độ tâm đài (độ)

    // ---- Vòng tròn cự ly và đường chia độ ----
    double maxRangeKm = 50.0; ///< cự ly tối đa của ra đa (km)
    RangeRingMode rangeRings = RangeRingMode::R5km;
    BearingLineMode bearingLines = BearingLineMode::D30;

    // ---- Quỹ đạo ----
    bool showTrackInfo = true;      ///< hiện thông tin quỹ đạo bên cạnh dấu quỹ đạo
    bool autoDeleteTrack = true;    ///< tự động xoá quỹ đạo khi mất cập nhật
    int autoDeleteSeconds = 60;     ///< số giây không nhận được cập nhật thì xoá

    // ---- Màu sắc các đối tượng ----
    QColor colorMarkPoint{ QStringLiteral("#00C8FF") }; ///< điểm dấu
    QColor colorTrack{ QStringLiteral("#FFD800") };     ///< quỹ đạo
    QColor colorTrackInfo{ QStringLiteral("#E0E0E0") }; ///< thông tin quỹ đạo
    QColor colorSweep{ QStringLiteral("#00E05A") };     ///< đường quét ra đa (xanh lá)

    /// Đường dẫn tệp cấu hình: <thư mục file chạy>/X-01.json
    static QString filePath();

    /// Đọc cấu hình; nếu chưa có tệp thì giữ nguyên toàn bộ giá trị mặc định.
    void load();

    /// Ghi cấu hình ra tệp .json. Trả về false nếu không ghi được.
    bool save() const;
};
