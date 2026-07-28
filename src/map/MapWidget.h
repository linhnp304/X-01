#pragma once

#include "map/MapTheme.h"

#include <QFont>
#include <QPointF>
#include <QWidget>

class AppSettings;
class MapData;

class QFrame;
class QSlider;
class QToolButton;

/// Panel 1 — màn hình hiển thị dữ liệu chính.
///
/// Tự vẽ toàn bộ bằng QPainter, không dùng thư viện bản đồ ngoài.
/// Phép chiếu tương đương (equirectangular): x = kinh độ, y = -vĩ độ.
class MapWidget : public QWidget
{
    Q_OBJECT

public:
    explicit MapWidget(QWidget *parent = nullptr);

    /// Gắn dữ liệu bản đồ đã nạp (đối tượng do MainWindow sở hữu).
    void setMapData(const MapData *data);

    /// Gắn cấu hình đang dùng (đối tượng do MainWindow sở hữu).
    void setSettings(const AppSettings *settings);

    /// Áp dụng lại cấu hình: dựng lại bảng màu theo độ sáng rồi vẽ lại.
    void applySettings();

    /// Đưa khung nhìn về đúng tâm đài, ở mức phóng vừa đủ thấy hết cự ly tối đa.
    void centerOnRadar();

    /// Phương vị đường quét ra đa, tính bằng độ (0 = hướng Bắc, tăng theo chiều kim đồng hồ).
    double sweepAzimuth() const { return m_sweepAzimuth; }
    void setSweepAzimuth(double deg);

signals:
    /// Con trỏ chuột di chuyển trên bản đồ (toạ độ địa lý tương ứng).
    void mouseGeoMoved(double lat, double lng);
    /// Con trỏ chuột rời khỏi bản đồ.
    void mouseGeoLeft();

protected:
    void paintEvent(QPaintEvent *event) override;
    void resizeEvent(QResizeEvent *event) override;
    void mousePressEvent(QMouseEvent *event) override;
    void mouseMoveEvent(QMouseEvent *event) override;
    void mouseReleaseEvent(QMouseEvent *event) override;
    void wheelEvent(QWheelEvent *event) override;
    void leaveEvent(QEvent *event) override;

private:
    // ---- Chuyển đổi toạ độ ----
    QPointF geoToScreen(double lat, double lng) const;
    void screenToGeo(const QPointF &p, double *lat, double *lng) const;

    /// Số pixel trên màn hình ứng với cự ly km theo phương ngang (kinh độ) và phương dọc (vĩ độ).
    /// Ở phép chiếu equirectangular, một vòng tròn cự ly thật trên mặt đất hiện lên
    /// dưới dạng hình elip hơi bè ngang, tỉ lệ 1/cos(vĩ độ tâm đài).
    double kmToPixelsX(double km) const;
    double kmToPixelsY(double km) const;

    // ---- Các bước vẽ ----
    void drawMapLayers(QPainter &p);
    void drawLabels(QPainter &p);
    void drawRangeRings(QPainter &p);
    void drawBearingLines(QPainter &p);
    void drawSweep(QPainter &p);
    void drawNoDataHint(QPainter &p);

    // ---- Thanh trượt zoom ở góc dưới bên phải ----
    void buildZoomBar();
    void layoutZoomBar();
    void syncZoomSlider();
    /// Đổi tỉ lệ hiện tại thành mức 0..100 trên thanh trượt (thang logarit).
    double scaleToZoomLevel(double scale) const;
    double zoomLevelToScale(double level) const;
    /// Phóng to/thu nhỏ quanh một điểm cố định trên màn hình.
    void zoomAt(const QPointF &anchor, double factor);

    const MapData *m_data = nullptr;
    const AppSettings *m_settings = nullptr;

    MapTheme m_theme;

    // Khung nhìn hiện tại
    double m_centerLat = 21.0285;
    double m_centerLng = 105.8542;
    double m_scale = 400.0; ///< số pixel trên 1 độ

    // Trạng thái kéo bản đồ bằng chuột trái
    bool m_mouseDown = false;
    bool m_dragging = false;
    QPointF m_dragStart;
    double m_dragCenterLat = 0.0;
    double m_dragCenterLng = 0.0;

    double m_sweepAzimuth = 0.0; ///< mặc định 0 độ = hướng Bắc

    /// Người dùng đã tự kéo hoặc phóng bản đồ lần nào chưa.
    /// Trước lúc đó, khung nhìn được canh lại theo tâm đài mỗi khi panel đổi kích thước.
    bool m_userChangedView = false;

    QFrame *m_zoomBar = nullptr;
    QSlider *m_zoomSlider = nullptr;
    QToolButton *m_zoomOutBtn = nullptr;
    QToolButton *m_zoomInBtn = nullptr;

    QFont m_placeFont;
    QFont m_airportFont;
    QFont m_routeFont;
};
