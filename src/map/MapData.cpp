#include "map/MapData.h"

#include "map/DbfReader.h"
#include "map/PrjProjection.h"

#include <QFile>
#include <QStringConverter>
#include <QTextStream>

#include <utility>

namespace {

/// Ngưỡng lược điểm (tính bằng độ) của từng mức chi tiết.
/// LOD 0 ~ 1 km, LOD 1 ~ 130 m, LOD 2 ~ 11 m — đủ mịn cho mức phóng to tối đa.
constexpr double kLodTolerance[kLodCount] = { 0.0100, 0.0012, 0.0001 };

/// Ngưỡng tỉ lệ (pixel trên 1 độ) để chuyển sang mức chi tiết cao hơn.
constexpr double kLodScaleStep0 = 120.0;  // dưới mức này dùng LOD 0
constexpr double kLodScaleStep1 = 1000.0; // dưới mức này dùng LOD 1, trên thì LOD 2

/// Đổi một điểm lat-lng sang toạ độ thế giới dùng khi vẽ: x = kinh độ, y = -vĩ độ.
inline QPointF toWorld(const QPointF &lngLat)
{
    return QPointF(lngLat.x(), -lngLat.y());
}

/// Lược bớt điểm của một đường theo khoảng cách tối thiểu (radial distance).
/// Bỏ những điểm cách điểm vừa giữ lại dưới ngưỡng tol; luôn giữ điểm đầu và điểm cuối
/// để đường không bị co ngắn hay hở nối.
QPolygonF simplify(const QPolygonF &src, double tol)
{
    if (src.size() <= 2 || tol <= 0.0)
        return src;

    const double tol2 = tol * tol;
    QPolygonF out;
    out.reserve(src.size());
    out << src.first();

    QPointF last = src.first();
    for (int i = 1; i < src.size() - 1; ++i) {
        const QPointF &p = src.at(i);
        const double dx = p.x() - last.x();
        const double dy = p.y() - last.y();
        if (dx * dx + dy * dy >= tol2) {
            out << p;
            last = p;
        }
    }
    out << src.last();
    return out;
}

/// Hình chữ nhật bao quanh một đường, dùng để loại nhanh phần ngoài khung nhìn.
QRectF boundsOf(const QPolygonF &poly)
{
    if (poly.isEmpty())
        return QRectF();
    double minX = poly.first().x(), maxX = minX;
    double minY = poly.first().y(), maxY = minY;
    for (const QPointF &p : poly) {
        minX = qMin(minX, p.x());
        maxX = qMax(maxX, p.x());
        minY = qMin(minY, p.y());
        maxY = qMax(maxY, p.y());
    }
    return QRectF(QPointF(minX, minY), QPointF(maxX, maxY));
}

} // namespace

// ---------------------------------------------------------------- LineLayer

void LineLayer::build(const QVector<ShpShape> &shapes, bool closed)
{
    m_closed = closed;
    for (int lod = 0; lod < kLodCount; ++lod) {
        m_parts[lod].clear();
        m_bounds[lod].clear();
    }

    for (const ShpShape &shape : shapes) {
        for (const QPolygonF &part : shape.parts) {
            if (part.size() < 2)
                continue;

            // Chuyển sang toạ độ thế giới một lần, rồi lược riêng cho từng mức chi tiết
            QPolygonF world;
            world.reserve(part.size());
            for (const QPointF &p : part)
                world << toWorld(p);

            for (int lod = 0; lod < kLodCount; ++lod) {
                QPolygonF simplified = simplify(world, kLodTolerance[lod]);
                if (simplified.size() < 2)
                    continue;
                m_bounds[lod].append(boundsOf(simplified));
                m_parts[lod].append(std::move(simplified));
            }
        }
    }
}

// ---------------------------------------------------------------- FillLayer

void FillLayer::build(const QVector<ShpShape> &shapes)
{
    for (int lod = 0; lod < kLodCount; ++lod) {
        m_path[lod] = QPainterPath();
        // Quy tắc Winding tương đương FillRule.Nonzero của bản mẫu C#:
        // các vòng ngược chiều nhau sẽ khoét lỗ đúng cách.
        m_path[lod].setFillRule(Qt::WindingFill);
    }

    for (const ShpShape &shape : shapes) {
        for (const QPolygonF &part : shape.parts) {
            if (part.size() < 3)
                continue;

            QPolygonF world;
            world.reserve(part.size());
            for (const QPointF &p : part)
                world << toWorld(p);

            for (int lod = 0; lod < kLodCount; ++lod) {
                const QPolygonF simplified = simplify(world, kLodTolerance[lod]);
                if (simplified.size() < 3)
                    continue;
                m_path[lod].addPolygon(simplified);
                m_path[lod].closeSubpath();
            }
        }
    }
}

// ---------------------------------------------------------------- MapData

int MapData::lodForScale(double pixelsPerDegree)
{
    if (pixelsPerDegree < kLodScaleStep0)
        return 0;
    if (pixelsPerDegree < kLodScaleStep1)
        return 1;
    return 2;
}

QVector<ShpShape> MapData::readAndProject(const QString &shpPath)
{
    QVector<ShpShape> shapes = ShapefileReader::read(shpPath);

    const std::unique_ptr<PrjProjection> proj = PrjProjection::load(shpPath);
    if (proj) {
        // Tệp dùng hệ toạ độ chiếu -> đổi từng điểm về (kinh độ, vĩ độ) tính bằng độ
        for (ShpShape &s : shapes)
            for (QPolygonF &part : s.parts)
                for (QPointF &p : part)
                    p = proj->toLngLat(p);
    }
    return shapes;
}

bool MapData::load(const QString &dir)
{
    const QString sep = QStringLiteral("/");

    // --- Lớp nền: đất liền + đảo (tô nền), Hoàng Sa, Trường Sa ---
    const QVector<ShpShape> vnm = readAndProject(dir + sep + QStringLiteral("VNM_adm1.shp"));
    const QVector<ShpShape> hoangSa = readAndProject(dir + sep + QStringLiteral("Hoang_Sa.shp"));
    const QVector<ShpShape> truongSa = readAndProject(dir + sep + QStringLiteral("Truong_Sa.shp"));

    QVector<ShpShape> landShapes;
    landShapes += vnm;
    landShapes += readAndProject(dir + sep + QStringLiteral("Land-Islands-2.shp"));
    landShapes += hoangSa;
    landShapes += truongSa;
    m_landFill.build(landShapes);

    // --- Địa phận tỉnh/thành phố (viền khép kín) ---
    m_provinceLines.build(vnm, true);

    // --- Ranh giới quốc gia + đường biên giới (đường mở) ---
    QVector<ShpShape> nation;
    nation += readAndProject(dir + sep + QStringLiteral("Ranhgoiquocgia.shp"));
    nation += readAndProject(dir + sep + QStringLiteral("Duongbiengioi.shp"));
    m_nationLines.build(nation, false);

    // --- Bờ biển (đường mở) ---
    m_coastLines.build(readAndProject(dir + sep + QStringLiteral("CoastLines.shp")), false);

    // --- Viền đảo Hoàng Sa / Trường Sa (khép kín) ---
    QVector<ShpShape> islands;
    islands += hoangSa;
    islands += truongSa;
    m_islandLines.build(islands, true);

    // --- Đường bay dân dụng + tên đường bay (đường mở) ---
    const QVector<ShpShape> routes = readAndProject(dir + sep + QStringLiteral("AirRoutes.shp"));
    m_airRouteLines.build(routes, false);

    const QStringList routeNames =
        DbfReader::readColumn(dir + sep + QStringLiteral("AirRoutes.dbf"), QStringLiteral("name"));
    m_airRouteLabels.clear();
    for (qsizetype i = 0; i < routes.size() && i < routeNames.size(); ++i) {
        if (routeNames.at(i).trimmed().isEmpty())
            continue;
        if (routes.at(i).parts.isEmpty())
            continue;
        const QPolygonF &part = routes.at(i).parts.first();
        if (part.isEmpty())
            continue;
        // Đặt nhãn ở giữa đường bay
        const QPointF mid = part.at(part.size() / 2);
        MapLabel lb;
        lb.text = routeNames.at(i);
        lb.lat = mid.y();
        lb.lng = mid.x();
        m_airRouteLabels.append(lb);
    }

    loadPlaces(dir + sep + QStringLiteral("Diadanh.txt"));
    loadAirports(dir + sep + QStringLiteral("Airport2.dat"));

    m_loaded = !landShapes.isEmpty();
    return m_loaded;
}

void MapData::loadPlaces(const QString &path)
{
    m_placeLabels.clear();

    // Diadanh.txt được lưu bằng UTF-16 (có BOM), mỗi dòng: tên <tab> vĩ độ <tab> kinh độ
    QFile f(path);
    if (f.open(QIODevice::ReadOnly)) {
        QTextStream ts(&f);
        ts.setEncoding(QStringConverter::Utf16);
        while (!ts.atEnd()) {
            const QStringList fields = ts.readLine().split(QLatin1Char('\t'));
            if (fields.size() < 3)
                continue;
            bool okLat = false, okLng = false;
            const double lat = fields.at(1).trimmed().toDouble(&okLat);
            const double lng = fields.at(2).trimmed().toDouble(&okLng);
            if (!okLat || !okLng)
                continue;
            MapLabel lb;
            lb.text = fields.at(0).trimmed();
            lb.lat = lat;
            lb.lng = lng;
            lb.withDot = true;
            m_placeLabels.append(lb);
        }
        f.close();
    }

    // Một số nhãn vùng biển không có trong tệp dữ liệu, bổ sung cố định (không kèm chấm)
    m_placeLabels.append({ QStringLiteral("VỊNH BẮC BỘ"), 19.7, 107.3, false });
    m_placeLabels.append({ QStringLiteral("BIỂN ĐÔNG"), 14.0, 111.0, false });
    m_placeLabels.append({ QStringLiteral("QĐ Hoàng Sa"), 16.470, 112.0, false });
    m_placeLabels.append({ QStringLiteral("QĐ Trường Sa"), 9.0, 114.0, false });
}

void MapData::loadAirports(const QString &path)
{
    m_airports.clear();

    // Airport2.dat lưu bằng UTF-8 (có BOM), các cột cách nhau bằng tab:
    // [0] tên, [1] vĩ độ, [2] kinh độ, ..., [13] hướng đường băng (độ)
    QFile f(path);
    if (!f.open(QIODevice::ReadOnly))
        return;

    QTextStream ts(&f);
    ts.setEncoding(QStringConverter::Utf8);
    while (!ts.atEnd()) {
        const QStringList fields = ts.readLine().split(QLatin1Char('\t'));
        if (fields.size() < 3)
            continue;
        bool okLat = false, okLng = false;
        const double lat = fields.at(1).trimmed().toDouble(&okLat);
        const double lng = fields.at(2).trimmed().toDouble(&okLng);
        if (!okLat || !okLng)
            continue;

        AirportInfo ap;
        // Bỏ ký tự BOM còn sót ở đầu dòng đầu tiên nếu có
        ap.name = fields.at(0).trimmed();
        ap.name.remove(QChar(0xFEFF));
        ap.lat = lat;
        ap.lng = lng;
        if (fields.size() > 13)
            ap.runwayHeading = fields.at(13).trimmed().toDouble();
        m_airports.append(ap);
    }
    f.close();
}
