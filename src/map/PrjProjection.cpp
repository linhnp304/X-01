#include "map/PrjProjection.h"

#include <QFile>
#include <QFileInfo>
#include <QRegularExpression>

#include <cmath>

namespace {

constexpr double kPi = 3.14159265358979323846;

double toRad(double deg) { return deg * kPi / 180.0; }
double toDeg(double rad) { return rad * 180.0 / kPi; }

} // namespace

std::unique_ptr<PrjProjection> PrjProjection::load(const QString &shpPath)
{
    // Tệp .prj nằm cùng thư mục, cùng tên gốc với .shp
    QFileInfo fi(shpPath);
    const QString prjPath = fi.absolutePath() + QLatin1Char('/') + fi.completeBaseName() + QStringLiteral(".prj");

    QFile f(prjPath);
    if (!f.open(QIODevice::ReadOnly | QIODevice::Text))
        return nullptr;
    const QString wkt = QString::fromUtf8(f.readAll());
    f.close();

    // Bắt đầu bằng GEOGCS -> toạ độ đã là độ, không cần chuyển đổi
    if (!wkt.trimmed().startsWith(QLatin1String("PROJCS"), Qt::CaseInsensitive))
        return nullptr;

    if (wkt.contains(QLatin1String("Lambert_Conformal_Conic"), Qt::CaseInsensitive))
        return std::unique_ptr<PrjProjection>(new PrjProjection(Kind::Lcc, wkt));
    if (wkt.contains(QLatin1String("Transverse_Mercator"), Qt::CaseInsensitive))
        return std::unique_ptr<PrjProjection>(new PrjProjection(Kind::Tm, wkt));

    return nullptr; // phép chiếu chưa hỗ trợ: giữ nguyên toạ độ
}

PrjProjection::PrjProjection(Kind kind, const QString &wkt)
    : m_kind(kind)
{
    m_a = 6378137.0;
    const double invF = param(wkt, QStringLiteral("SPHEROID"), 2, 298.257223563);
    const double flat = 1.0 / invF;
    m_e2 = flat * (2.0 - flat);
    m_e = std::sqrt(m_e2);

    m_unit = projectedUnit(wkt);
    m_fe = param(wkt, QStringLiteral("false_easting"), 1, 0.0) * m_unit;
    m_fn = param(wkt, QStringLiteral("false_northing"), 1, 0.0) * m_unit;
    m_lng0 = toRad(param(wkt, QStringLiteral("central_meridian"), 1, 0.0));
    const double lat0 = toRad(param(wkt, QStringLiteral("latitude_of_origin"), 1, 0.0));

    if (kind == Kind::Lcc) {
        // Hai vĩ tuyến chuẩn của phép chiếu nón đồng góc
        const double sp1 = toRad(param(wkt, QStringLiteral("standard_parallel_1"), 1, 15.0));
        const double sp2 = toRad(param(wkt, QStringLiteral("standard_parallel_2"), 1, 45.0));
        const double m1 = m(sp1);
        const double m2 = m(sp2);
        const double t0 = t(lat0);
        const double t1 = t(sp1);
        const double t2 = t(sp2);
        m_n = (std::log(m1) - std::log(m2)) / (std::log(t1) - std::log(t2));
        m_f = m1 / (m_n * std::pow(t1, m_n));
        m_rho0 = m_a * m_f * std::pow(t0, m_n);
    } else {
        m_k0 = param(wkt, QStringLiteral("scale_factor"), 1, 1.0);
        m_m0 = meridianArc(lat0);
        const double sq = std::sqrt(1.0 - m_e2);
        m_e1 = (1.0 - sq) / (1.0 + sq);
        m_ep2 = m_e2 / (1.0 - m_e2);
    }
}

QPointF PrjProjection::toLngLat(const QPointF &p) const
{
    return m_kind == Kind::Lcc ? inverseLcc(p.x() * m_unit, p.y() * m_unit)
                               : inverseTm(p.x() * m_unit, p.y() * m_unit);
}

// ---------------- Lambert Conformal Conic 2SP (EPSG 9802) ----------------

QPointF PrjProjection::inverseLcc(double x, double y) const
{
    const double xd = x - m_fe;
    const double yd = m_rho0 - (y - m_fn);
    const double sign = (m_n < 0.0) ? -1.0 : 1.0;
    const double rho = sign * std::sqrt(xd * xd + yd * yd);
    const double tVal = std::pow(rho / (m_a * m_f), 1.0 / m_n);
    const double theta = std::atan2(xd, yd);
    const double lng = theta / m_n + m_lng0;
    const double lat = phiFromT(tVal);
    return QPointF(toDeg(lng), toDeg(lat));
}

// ---------------- Transverse Mercator (EPSG 9807) ----------------

QPointF PrjProjection::inverseTm(double x, double y) const
{
    const double mArc = m_m0 + (y - m_fn) / m_k0;
    const double mu = mArc / (m_a * (1.0 - m_e2 / 4.0 - 3.0 * m_e2 * m_e2 / 64.0
                                     - 5.0 * m_e2 * m_e2 * m_e2 / 256.0));
    const double e1 = m_e1;

    // Vĩ độ chân (footprint latitude) — khai triển chuỗi theo mu
    const double phi1 = mu
        + (3.0 * e1 / 2.0 - 27.0 * std::pow(e1, 3) / 32.0) * std::sin(2.0 * mu)
        + (21.0 * e1 * e1 / 16.0 - 55.0 * std::pow(e1, 4) / 32.0) * std::sin(4.0 * mu)
        + 151.0 * std::pow(e1, 3) / 96.0 * std::sin(6.0 * mu)
        + 1097.0 * std::pow(e1, 4) / 512.0 * std::sin(8.0 * mu);

    const double sin1 = std::sin(phi1);
    const double cos1 = std::cos(phi1);
    const double tan1 = std::tan(phi1);
    const double c1 = m_ep2 * cos1 * cos1;
    const double t1 = tan1 * tan1;
    const double n1 = m_a / std::sqrt(1.0 - m_e2 * sin1 * sin1);
    const double r1 = m_a * (1.0 - m_e2) / std::pow(1.0 - m_e2 * sin1 * sin1, 1.5);
    const double d = (x - m_fe) / (n1 * m_k0);

    const double lat = phi1 - n1 * tan1 / r1
        * (d * d / 2.0
           - (5.0 + 3.0 * t1 + 10.0 * c1 - 4.0 * c1 * c1 - 9.0 * m_ep2) * std::pow(d, 4) / 24.0
           + (61.0 + 90.0 * t1 + 298.0 * c1 + 45.0 * t1 * t1 - 252.0 * m_ep2 - 3.0 * c1 * c1)
               * std::pow(d, 6) / 720.0);

    const double lng = m_lng0
        + (d - (1.0 + 2.0 * t1 + c1) * std::pow(d, 3) / 6.0
           + (5.0 - 2.0 * c1 + 28.0 * t1 - 3.0 * c1 * c1 + 8.0 * m_ep2 + 24.0 * t1 * t1)
               * std::pow(d, 5) / 120.0) / cos1;

    return QPointF(toDeg(lng), toDeg(lat));
}

// ---------------- Hàm phụ ----------------

double PrjProjection::m(double phi) const
{
    return std::cos(phi) / std::sqrt(1.0 - m_e2 * std::sin(phi) * std::sin(phi));
}

double PrjProjection::t(double phi) const
{
    return std::tan(kPi / 4.0 - phi / 2.0)
        / std::pow((1.0 - m_e * std::sin(phi)) / (1.0 + m_e * std::sin(phi)), m_e / 2.0);
}

double PrjProjection::phiFromT(double tVal) const
{
    // Lặp hội tụ nhanh: 8 vòng là quá đủ cho độ chính xác dưới 1 mm
    double phi = kPi / 2.0 - 2.0 * std::atan(tVal);
    for (int i = 0; i < 8; ++i) {
        const double es = m_e * std::sin(phi);
        phi = kPi / 2.0 - 2.0 * std::atan(tVal * std::pow((1.0 - es) / (1.0 + es), m_e / 2.0));
    }
    return phi;
}

double PrjProjection::meridianArc(double phi) const
{
    return m_a
        * ((1.0 - m_e2 / 4.0 - 3.0 * m_e2 * m_e2 / 64.0 - 5.0 * std::pow(m_e2, 3) / 256.0) * phi
           - (3.0 * m_e2 / 8.0 + 3.0 * m_e2 * m_e2 / 32.0 + 45.0 * std::pow(m_e2, 3) / 1024.0)
               * std::sin(2.0 * phi)
           + (15.0 * m_e2 * m_e2 / 256.0 + 45.0 * std::pow(m_e2, 3) / 1024.0) * std::sin(4.0 * phi)
           - 35.0 * std::pow(m_e2, 3) / 3072.0 * std::sin(6.0 * phi));
}

double PrjProjection::projectedUnit(const QString &wkt)
{
    // UNIT xuất hiện nhiều lần trong WKT; cái CUỐI CÙNG là đơn vị của hệ chiếu
    // (Meter/Kilometer), các cái trước là đơn vị góc của hệ toạ độ địa lý.
    static const QRegularExpression re(QStringLiteral("UNIT\\[\"([^\"]+)\",([0-9.eE+-]+)"),
                                       QRegularExpression::CaseInsensitiveOption);
    QRegularExpressionMatch last;
    auto it = re.globalMatch(wkt);
    while (it.hasNext())
        last = it.next();

    if (last.hasMatch()) {
        const QString name = last.captured(1);
        bool ok = false;
        const double v = last.captured(2).toDouble(&ok);
        if (ok && !name.contains(QLatin1String("degree"), Qt::CaseInsensitive))
            return v;
    }
    return 1.0;
}

double PrjProjection::param(const QString &wkt, const QString &name, int group, double fallback)
{
    const bool isSpheroid = (name.compare(QLatin1String("SPHEROID"), Qt::CaseInsensitive) == 0);
    const QString pattern = isSpheroid
        ? QStringLiteral("SPHEROID\\[\"[^\"]*\",([0-9.eE+-]+),([0-9.eE+-]+)")
        : QStringLiteral("PARAMETER\\[\"%1\",([0-9.eE+-]+)").arg(QRegularExpression::escape(name));

    const QRegularExpression re(pattern, QRegularExpression::CaseInsensitiveOption);
    const QRegularExpressionMatch mt = re.match(wkt);
    if (!mt.hasMatch())
        return fallback;

    const int g = qMin(group, mt.lastCapturedIndex());
    bool ok = false;
    const double v = mt.captured(g).toDouble(&ok);
    return ok ? v : fallback;
}
