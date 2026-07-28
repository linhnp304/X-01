#include "AppSettings.h"

#include <QCoreApplication>
#include <QFile>
#include <QJsonDocument>
#include <QJsonObject>
#include <QSaveFile>

namespace {

// Chuỗi đại diện cho từng chế độ trong tệp .json — để người dùng đọc/sửa tay được.

QString ringToString(RangeRingMode m)
{
    switch (m) {
    case RangeRingMode::R5km:  return QStringLiteral("5km");
    case RangeRingMode::R1km:  return QStringLiteral("1km");
    case RangeRingMode::R05km: return QStringLiteral("0.5km");
    case RangeRingMode::R01km: return QStringLiteral("0.1km");
    case RangeRingMode::Off:   break;
    }
    return QStringLiteral("off");
}

RangeRingMode ringFromString(const QString &s, RangeRingMode fallback)
{
    if (s == QLatin1String("5km"))   return RangeRingMode::R5km;
    if (s == QLatin1String("1km"))   return RangeRingMode::R1km;
    if (s == QLatin1String("0.5km")) return RangeRingMode::R05km;
    if (s == QLatin1String("0.1km")) return RangeRingMode::R01km;
    if (s == QLatin1String("off"))   return RangeRingMode::Off;
    return fallback;
}

QString bearingToString(BearingLineMode m)
{
    switch (m) {
    case BearingLineMode::D30: return QStringLiteral("30");
    case BearingLineMode::D10: return QStringLiteral("10");
    case BearingLineMode::D5:  return QStringLiteral("5");
    case BearingLineMode::Off: break;
    }
    return QStringLiteral("off");
}

BearingLineMode bearingFromString(const QString &s, BearingLineMode fallback)
{
    if (s == QLatin1String("30"))  return BearingLineMode::D30;
    if (s == QLatin1String("10"))  return BearingLineMode::D10;
    if (s == QLatin1String("5"))   return BearingLineMode::D5;
    if (s == QLatin1String("off")) return BearingLineMode::Off;
    return fallback;
}

/// Đọc màu dạng chuỗi "#RRGGBB"; giữ nguyên giá trị mặc định nếu chuỗi không hợp lệ.
QColor colorFrom(const QJsonObject &o, const QString &key, const QColor &fallback)
{
    const QColor c(o.value(key).toString());
    return c.isValid() ? c : fallback;
}

} // namespace

QString AppSettings::filePath()
{
    return QCoreApplication::applicationDirPath() + QStringLiteral("/X-01.json");
}

void AppSettings::load()
{
    QFile f(filePath());
    if (!f.open(QIODevice::ReadOnly))
        return; // chưa có tệp cấu hình: dùng toàn bộ giá trị mặc định

    const QJsonDocument doc = QJsonDocument::fromJson(f.readAll());
    f.close();
    if (!doc.isObject())
        return;

    const QJsonObject o = doc.object();

    // Nền bản đồ
    const QJsonObject map = o.value(QStringLiteral("banDo")).toObject();
    mapBrightness  = qBound(1, map.value(QStringLiteral("doSang")).toInt(mapBrightness), 10);
    showAirRoutes  = map.value(QStringLiteral("hienDuongBayDanDung")).toBool(showAirRoutes);
    showAirports   = map.value(QStringLiteral("hienSanBay")).toBool(showAirports);
    showPlaceNames = map.value(QStringLiteral("hienTenDiaDanh")).toBool(showPlaceNames);
    showProvinces  = map.value(QStringLiteral("hienDiaPhanTinh")).toBool(showProvinces);

    // Tâm đài
    const QJsonObject radar = o.value(QStringLiteral("tamDai")).toObject();
    radarLat = qBound(-90.0, radar.value(QStringLiteral("lat")).toDouble(radarLat), 90.0);
    radarLng = qBound(-180.0, radar.value(QStringLiteral("lng")).toDouble(radarLng), 180.0);

    // Vòng tròn cự ly và đường chia độ
    const QJsonObject grid = o.value(QStringLiteral("luoiCuLy")).toObject();
    maxRangeKm = grid.value(QStringLiteral("cuLyToiDaKm")).toDouble(maxRangeKm);
    if (maxRangeKm < 1.0 || maxRangeKm > 2000.0)
        maxRangeKm = 50.0; // giá trị vô lý trong tệp -> quay về mặc định
    rangeRings = ringFromString(grid.value(QStringLiteral("vongTronCuLy")).toString(), rangeRings);
    bearingLines = bearingFromString(grid.value(QStringLiteral("duongChiaDo")).toString(), bearingLines);

    // Quỹ đạo
    const QJsonObject track = o.value(QStringLiteral("quyDao")).toObject();
    showTrackInfo     = track.value(QStringLiteral("hienThongTin")).toBool(showTrackInfo);
    autoDeleteTrack   = track.value(QStringLiteral("tuDongXoa")).toBool(autoDeleteTrack);
    autoDeleteSeconds = qBound(1, track.value(QStringLiteral("soGiayMatCapNhat")).toInt(autoDeleteSeconds), 3600);

    // Màu sắc
    const QJsonObject colors = o.value(QStringLiteral("mauSac")).toObject();
    colorMarkPoint = colorFrom(colors, QStringLiteral("diemDau"), colorMarkPoint);
    colorTrack     = colorFrom(colors, QStringLiteral("quyDao"), colorTrack);
    colorTrackInfo = colorFrom(colors, QStringLiteral("thongTinQuyDao"), colorTrackInfo);
    colorSweep     = colorFrom(colors, QStringLiteral("duongQuet"), colorSweep);
}

bool AppSettings::save() const
{
    QJsonObject map;
    map[QStringLiteral("doSang")] = mapBrightness;
    map[QStringLiteral("hienDuongBayDanDung")] = showAirRoutes;
    map[QStringLiteral("hienSanBay")] = showAirports;
    map[QStringLiteral("hienTenDiaDanh")] = showPlaceNames;
    map[QStringLiteral("hienDiaPhanTinh")] = showProvinces;

    QJsonObject radar;
    radar[QStringLiteral("lat")] = radarLat;
    radar[QStringLiteral("lng")] = radarLng;

    QJsonObject grid;
    grid[QStringLiteral("cuLyToiDaKm")] = maxRangeKm;
    grid[QStringLiteral("vongTronCuLy")] = ringToString(rangeRings);
    grid[QStringLiteral("duongChiaDo")] = bearingToString(bearingLines);

    QJsonObject track;
    track[QStringLiteral("hienThongTin")] = showTrackInfo;
    track[QStringLiteral("tuDongXoa")] = autoDeleteTrack;
    track[QStringLiteral("soGiayMatCapNhat")] = autoDeleteSeconds;

    QJsonObject colors;
    colors[QStringLiteral("diemDau")] = colorMarkPoint.name(QColor::HexRgb).toUpper();
    colors[QStringLiteral("quyDao")] = colorTrack.name(QColor::HexRgb).toUpper();
    colors[QStringLiteral("thongTinQuyDao")] = colorTrackInfo.name(QColor::HexRgb).toUpper();
    colors[QStringLiteral("duongQuet")] = colorSweep.name(QColor::HexRgb).toUpper();

    QJsonObject root;
    root[QStringLiteral("banDo")] = map;
    root[QStringLiteral("tamDai")] = radar;
    root[QStringLiteral("luoiCuLy")] = grid;
    root[QStringLiteral("quyDao")] = track;
    root[QStringLiteral("mauSac")] = colors;

    // QSaveFile: ghi ra tệp tạm rồi mới thay thế, tránh hỏng cấu hình nếu mất điện giữa chừng
    QSaveFile f(filePath());
    if (!f.open(QIODevice::WriteOnly | QIODevice::Text))
        return false;
    f.write(QJsonDocument(root).toJson(QJsonDocument::Indented));
    return f.commit();
}
