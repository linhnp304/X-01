#pragma once

#include "map/ShapefileReader.h"

#include <QPainterPath>
#include <QPolygonF>
#include <QRectF>
#include <QString>
#include <QVector>

/// Nhãn chữ trên bản đồ (tên địa danh, tên đường bay...).
struct MapLabel
{
    QString text;
    double lat = 0.0;
    double lng = 0.0;
    bool withDot = false; ///< có vẽ kèm chấm đánh dấu vị trí hay không
};

/// Thông tin một sân bay.
struct AirportInfo
{
    QString name;
    double lat = 0.0;
    double lng = 0.0;
    double runwayHeading = 0.0; ///< hướng đường băng, độ (0 = hướng Bắc)
};

/// Số mức chi tiết (LOD) mà mỗi lớp hình học được dựng sẵn.
/// Dữ liệu gốc rất nặng (riêng VNM_adm1.shp đã hơn 6 MB), vẽ toàn bộ mỗi khung
/// hình sẽ giật khi kéo/zoom. Vì vậy mỗi lớp được lược bớt điểm thành 3 mức và
/// khi vẽ chỉ chọn mức đủ mịn so với tỉ lệ hiện tại.
constexpr int kLodCount = 3;

/// Lớp hình dạng đường (viền tỉnh, bờ biển, đường bay...).
/// Giữ riêng từng "part" kèm hình chữ nhật bao để loại nhanh những phần
/// nằm ngoài khung nhìn (rất hiệu quả khi phóng to).
class LineLayer
{
public:
    /// closed = true khi mỗi part là một vòng khép kín (viền tỉnh, viền đảo);
    /// false khi là đường mở (bờ biển, biên giới, đường bay).
    void build(const QVector<ShpShape> &shapes, bool closed);

    const QVector<QPolygonF> &parts(int lod) const { return m_parts[clampLod(lod)]; }
    const QVector<QRectF> &bounds(int lod) const { return m_bounds[clampLod(lod)]; }
    bool isClosed() const { return m_closed; }

private:
    static int clampLod(int lod) { return qBound(0, lod, kLodCount - 1); }

    QVector<QPolygonF> m_parts[kLodCount];
    QVector<QRectF> m_bounds[kLodCount];
    bool m_closed = false;
};

/// Lớp hình dạng cần tô nền (đất liền + đảo).
/// Phải gom vào một QPainterPath duy nhất với quy tắc tô Winding thì các lỗ
/// (hồ, vùng lõm) mới được trừ đúng.
class FillLayer
{
public:
    void build(const QVector<ShpShape> &shapes);

    const QPainterPath &path(int lod) const { return m_path[qBound(0, lod, kLodCount - 1)]; }

private:
    QPainterPath m_path[kLodCount];
};

/// Nạp toàn bộ dữ liệu bản đồ số từ thư mục MapChuan và dựng sẵn hình học
/// theo toạ độ thế giới (x = kinh độ, y = -vĩ độ) để vẽ bằng QPainter.
class MapData
{
public:
    /// Nạp dữ liệu từ thư mục dir (thường là <thư mục file chạy>/MapChuan).
    /// Trả về false nếu không tìm thấy lớp nền chính — khi đó phần mềm vẫn chạy
    /// nhưng nền bản đồ sẽ trống.
    bool load(const QString &dir);

    bool isLoaded() const { return m_loaded; }

    const FillLayer &landFill() const { return m_landFill; }
    /// Sông ngòi — dữ liệu là vùng (lòng sông) chứ không phải đường tim sông.
    const FillLayer &rivers() const { return m_rivers; }
    const LineLayer &provinceLines() const { return m_provinceLines; }
    const LineLayer &nationLines() const { return m_nationLines; }
    const LineLayer &coastLines() const { return m_coastLines; }
    const LineLayer &islandLines() const { return m_islandLines; }
    const LineLayer &airRouteLines() const { return m_airRouteLines; }

    const QVector<MapLabel> &placeLabels() const { return m_placeLabels; }
    const QVector<MapLabel> &airRouteLabels() const { return m_airRouteLabels; }
    const QVector<AirportInfo> &airports() const { return m_airports; }

    /// Chọn mức chi tiết phù hợp với tỉ lệ hiện tại (số pixel trên 1 độ).
    static int lodForScale(double pixelsPerDegree);

private:
    void loadPlaces(const QString &path);
    void loadAirports(const QString &path);

    /// Đọc shapefile và quy đổi về lat-lng độ nếu tệp dùng hệ toạ độ chiếu (.prj).
    static QVector<ShpShape> readAndProject(const QString &shpPath);

    bool m_loaded = false;

    FillLayer m_landFill;
    FillLayer m_rivers;
    LineLayer m_provinceLines;
    LineLayer m_nationLines;
    LineLayer m_coastLines;
    LineLayer m_islandLines;
    LineLayer m_airRouteLines;

    QVector<MapLabel> m_placeLabels;
    QVector<MapLabel> m_airRouteLabels;
    QVector<AirportInfo> m_airports;
};
