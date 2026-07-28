#pragma once

#include <QPointF>
#include <QString>

#include <memory>

/// Đọc tệp .prj (định dạng WKT) đi kèm shapefile và chuyển toạ độ chiếu về lat-lng (độ).
///
/// Bộ dữ liệu MapChuan dùng 3 nhóm:
///   - GEOGCS  : toạ độ đã là độ, giữ nguyên (VNM_adm1, AirRoutes)
///   - Lambert Conformal Conic 2SP (CoastLines, Duongbiengioi, Ranhgoiquocgia, Land-Islands-2)
///   - Transverse Mercator         (Hoang_Sa, Truong_Sa)
class PrjProjection
{
public:
    /// Đọc .prj tương ứng với một shapefile.
    /// Trả về nullptr nếu toạ độ đã là lat-lng độ, hoặc phép chiếu chưa được hỗ trợ.
    static std::unique_ptr<PrjProjection> load(const QString &shpPath);

    /// Chuyển 1 điểm chiếu (x, y theo đơn vị khai báo trong .prj) về (kinh độ, vĩ độ) tính bằng độ.
    QPointF toLngLat(const QPointF &p) const;

private:
    enum class Kind { Lcc, Tm };

    PrjProjection(Kind kind, const QString &wkt);

    QPointF inverseLcc(double x, double y) const;
    QPointF inverseTm(double x, double y) const;

    // Hàm phụ trợ của phép chiếu nón/hình trụ
    double m(double phi) const;
    double t(double phi) const;
    double phiFromT(double tVal) const;
    double meridianArc(double phi) const;

    /// Lấy hệ số quy đổi đơn vị của hệ chiếu về mét (UNIT cuối cùng trong WKT).
    static double projectedUnit(const QString &wkt);
    /// Lấy giá trị PARAMETER["name",value] (hoặc tham số của SPHEROID) trong WKT.
    static double param(const QString &wkt, const QString &name, int group, double fallback);

    Kind m_kind;

    double m_a = 6378137.0; // bán trục lớn ellipsoid (m)
    double m_e = 0.0;       // độ lệch tâm
    double m_e2 = 0.0;      // bình phương độ lệch tâm
    double m_unit = 1.0;    // hệ số đổi đơn vị hệ chiếu về mét
    double m_fe = 0.0;      // false easting (m)
    double m_fn = 0.0;      // false northing (m)
    double m_lng0 = 0.0;    // kinh tuyến trục (rad)

    // Tham số riêng của Lambert Conformal Conic
    double m_n = 0.0;
    double m_f = 0.0;
    double m_rho0 = 0.0;

    // Tham số riêng của Transverse Mercator
    double m_k0 = 1.0;
    double m_m0 = 0.0;
    double m_e1 = 0.0;
    double m_ep2 = 0.0;
};
