#include "map/MapWidget.h"

#include "AppSettings.h"
#include "Theme.h"
#include "map/MapData.h"

#include <QFrame>
#include <QHBoxLayout>
#include <QMouseEvent>
#include <QPainter>
#include <QPainterPath>
#include <QResizeEvent>
#include <QSignalBlocker>
#include <QSlider>
#include <QToolButton>
#include <QWheelEvent>

#include <cmath>

namespace {

constexpr double kPi = 3.14159265358979323846;

/// Số km ứng với 1 độ vĩ tuyến (giá trị trung bình, đủ chính xác cho cự ly vài trăm km).
constexpr double kKmPerDegreeLat = 111.32;

// Giới hạn tỉ lệ hiển thị (số pixel trên 1 độ).
// 8  -> nhìn được cả khu vực Đông Nam Á
// 12000 -> khoảng 9 m trên 1 pixel, đủ mịn cho chế độ vòng tròn 0,1 km
constexpr double kMinScale = 8.0;
constexpr double kMaxScale = 12000.0;

/// Khoảng cách tối thiểu giữa hai vòng tròn (pixel) để còn đáng vẽ.
/// Dưới ngưỡng này các vòng dính vào nhau thành một mảng đặc, vừa xấu vừa chậm.
constexpr double kMinRingSpacingPx = 3.0;

/// Vùng đệm quanh khung nhìn (pixel) khi lọc nhãn, để nhãn sát mép không nhấp nháy.
constexpr double kLabelMargin = 40.0;

double deg2rad(double d) { return d * kPi / 180.0; }

} // namespace

MapWidget::MapWidget(QWidget *parent)
    : QWidget(parent)
{
    setAutoFillBackground(false);
    setMouseTracking(true);
    setFocusPolicy(Qt::StrongFocus);
    setCursor(Qt::CrossCursor);

    m_theme = MapTheme::fromBrightness(1);

    m_placeFont.setFamilies(Theme::fontFamilies());
    m_placeFont.setPointSize(9);
    m_airportFont = m_placeFont;
    m_routeFont = m_placeFont;
    m_routeFont.setPointSize(8);

    buildZoomBar();
}

void MapWidget::setMapData(const MapData *data)
{
    m_data = data;
    update();
}

void MapWidget::setSettings(const AppSettings *settings)
{
    m_settings = settings;
    applySettings();
}

void MapWidget::applySettings()
{
    if (m_settings)
        m_theme = MapTheme::fromBrightness(m_settings->mapBrightness);
    update();
}

void MapWidget::setSweepAzimuth(double deg)
{
    // Chuẩn hoá về khoảng [0, 360)
    double a = std::fmod(deg, 360.0);
    if (a < 0.0)
        a += 360.0;
    if (qFuzzyCompare(a + 1.0, m_sweepAzimuth + 1.0))
        return;
    m_sweepAzimuth = a;
    update();
}

void MapWidget::centerOnRadar()
{
    if (!m_settings)
        return;

    m_centerLat = m_settings->radarLat;
    m_centerLng = m_settings->radarLng;

    // Chọn tỉ lệ sao cho khung nhìn cao khoảng 5 lần cự ly tối đa:
    // vòng tròn ngoài cùng chiếm khoảng 2/5 chiều cao, còn lại là nền bản đồ xung quanh.
    const double viewHeightDeg = 5.0 * m_settings->maxRangeKm / kKmPerDegreeLat;
    const double h = qMax(1, height());
    if (viewHeightDeg > 0.0)
        m_scale = qBound(kMinScale, h / viewHeightDeg, kMaxScale);

    syncZoomSlider();
    update();
}

// ---------------------------------------------------------- Chuyển đổi toạ độ

QPointF MapWidget::geoToScreen(double lat, double lng) const
{
    return QPointF((lng - m_centerLng) * m_scale + width() / 2.0,
                   (m_centerLat - lat) * m_scale + height() / 2.0);
}

void MapWidget::screenToGeo(const QPointF &p, double *lat, double *lng) const
{
    if (lat)
        *lat = m_centerLat - (p.y() - height() / 2.0) / m_scale;
    if (lng)
        *lng = m_centerLng + (p.x() - width() / 2.0) / m_scale;
}

double MapWidget::kmToPixelsY(double km) const
{
    return km / kKmPerDegreeLat * m_scale;
}

double MapWidget::kmToPixelsX(double km) const
{
    // 1 độ kinh tuyến ngắn dần khi lên vĩ độ cao -> quy đổi chia thêm cho cos(vĩ độ)
    const double lat = m_settings ? m_settings->radarLat : m_centerLat;
    const double c = std::cos(deg2rad(lat));
    if (std::abs(c) < 1e-6)
        return kmToPixelsY(km);
    return km / (kKmPerDegreeLat * c) * m_scale;
}

// ---------------------------------------------------------- Vẽ

void MapWidget::paintEvent(QPaintEvent *event)
{
    Q_UNUSED(event)

    QPainter p(this);
    p.setRenderHint(QPainter::Antialiasing, true);
    p.setRenderHint(QPainter::TextAntialiasing, true);

    // Nền biển phủ toàn bộ panel
    p.fillRect(rect(), m_theme.sea);

    if (m_data && m_data->isLoaded()) {
        drawMapLayers(p);
        drawLabels(p);
    } else {
        drawNoDataHint(p);
    }

    // Lớp phủ của ra đa luôn nằm trên nền bản đồ
    drawRangeRings(p);
    drawBearingLines(p);
    drawSweep(p);
}

void MapWidget::drawMapLayers(QPainter &p)
{
    const int lod = MapData::lodForScale(m_scale);

    // Khung nhìn quy về toạ độ thế giới (x = kinh độ, y = -vĩ độ) để loại phần ngoài màn hình
    const double halfW = width() / 2.0 / m_scale;
    const double halfH = height() / 2.0 / m_scale;
    const QRectF viewWorld(m_centerLng - halfW, -m_centerLat - halfH, halfW * 2.0, halfH * 2.0);

    p.save();
    // Ma trận đưa toạ độ thế giới về toạ độ màn hình.
    // Dùng combine = true để GIỮ ma trận sẵn có của QPainter (phần dịch chuyển
    // vị trí widget trong backing store, hệ số màn hình HiDPI); nếu thay thế
    // hẳn thì bản đồ sẽ vẽ lệch chỗ trên các máy có tỉ lệ hiển thị khác 100%.
    p.setWorldTransform(QTransform(m_scale, 0.0, 0.0, m_scale,
                                   width() / 2.0 - m_centerLng * m_scale,
                                   height() / 2.0 + m_centerLat * m_scale),
                        true);

    // Nét bút dùng chế độ "cosmetic": độ dày tính bằng pixel màn hình,
    // không bị ma trận phóng to làm dày lên khi zoom.
    auto makePen = [](const QColor &c, double widthPx) {
        QPen pen(c);
        pen.setCosmetic(true);
        pen.setWidthF(widthPx);
        pen.setJoinStyle(Qt::RoundJoin);
        pen.setCapStyle(Qt::RoundCap);
        return pen;
    };

    /// Vẽ một lớp đường, bỏ qua những phần nằm ngoài khung nhìn.
    auto drawLines = [&](const LineLayer &layer, const QPen &pen) {
        p.setPen(pen);
        p.setBrush(Qt::NoBrush);
        const QVector<QPolygonF> &parts = layer.parts(lod);
        const QVector<QRectF> &bounds = layer.bounds(lod);
        for (qsizetype i = 0; i < parts.size(); ++i) {
            if (!bounds.at(i).intersects(viewWorld))
                continue;
            if (layer.isClosed())
                p.drawPolygon(parts.at(i));
            else
                p.drawPolyline(parts.at(i));
        }
    };

    // Nền đất liền + đảo
    p.setPen(Qt::NoPen);
    p.setBrush(m_theme.land);
    p.drawPath(m_data->landFill().path(lod));

    const AppSettings *s = m_settings;

    // Sông ngòi: vẽ ngay trên nền đất, dưới các lớp ranh giới.
    // Dữ liệu là vùng lòng sông nên vừa tô nền vừa kẻ viền mảnh cùng màu — khi thu
    // nhỏ, lòng sông hẹp hơn 1 pixel sẽ biến mất nếu chỉ tô, nét viền giữ cho
    // dòng sông vẫn liền mạch ở mọi mức phóng.
    if (!s || s->showRivers) {
        p.setPen(makePen(m_theme.river, 0.9));
        p.setBrush(m_theme.river);
        p.drawPath(m_data->rivers().path(lod));
    }

    if (!s || s->showProvinces)
        drawLines(m_data->provinceLines(), makePen(m_theme.province, 1.0));
    if (!s || s->showAirRoutes)
        drawLines(m_data->airRouteLines(), makePen(m_theme.airRoute, 1.0));

    drawLines(m_data->nationLines(), makePen(m_theme.nation, 1.6));
    drawLines(m_data->coastLines(), makePen(m_theme.coast, 1.4));
    drawLines(m_data->islandLines(), makePen(m_theme.coast, 1.0));

    p.restore();
}

void MapWidget::drawLabels(QPainter &p)
{
    const AppSettings *s = m_settings;

    // Vùng hợp lệ để hiện nhãn (rộng hơn panel một chút)
    const QRectF view(-kLabelMargin, -kLabelMargin,
                      width() + 2 * kLabelMargin, height() + 2 * kLabelMargin);

    // --- Tên địa danh (kèm chấm đánh dấu) ---
    if ((!s || s->showPlaceNames) && m_scale > 18.0) {
        p.setFont(m_placeFont);
        for (const MapLabel &lb : m_data->placeLabels()) {
            const QPointF pt = geoToScreen(lb.lat, lb.lng);
            if (!view.contains(pt))
                continue;
            if (lb.withDot) {
                p.setPen(Qt::NoPen);
                p.setBrush(m_theme.placeDot);
                p.drawEllipse(pt, 2.0, 2.0);
            }
            p.setPen(m_theme.label);
            p.drawText(QPointF(pt.x() + 4.0, pt.y() - 2.0), lb.text);
        }
    }

    // --- Tên đường bay dân dụng ---
    if ((!s || s->showAirRoutes) && m_scale > 30.0) {
        p.setFont(m_routeFont);
        p.setPen(m_theme.airRouteLabel);
        for (const MapLabel &lb : m_data->airRouteLabels()) {
            const QPointF pt = geoToScreen(lb.lat, lb.lng);
            if (!view.contains(pt))
                continue;
            p.drawText(QPointF(pt.x() + 2.0, pt.y() + 10.0), lb.text);
        }
    }

    // --- Sân bay: vòng tròn + vạch đường băng theo hướng ---
    if ((!s || s->showAirports) && m_scale > 18.0) {
        p.setFont(m_airportFont);
        QPen pen(m_theme.airport);
        pen.setWidthF(1.4);
        for (const AirportInfo &ap : m_data->airports()) {
            const QPointF pt = geoToScreen(ap.lat, ap.lng);
            if (!view.contains(pt))
                continue;
            p.setPen(pen);
            p.setBrush(Qt::NoBrush);
            p.drawEllipse(pt, 5.0, 5.0);
            // Hướng đường băng: 0 độ = hướng Bắc, quay theo chiều kim đồng hồ
            const double a = deg2rad(ap.runwayHeading);
            const QPointF d(std::sin(a) * 7.0, -std::cos(a) * 7.0);
            p.drawLine(pt - d, pt + d);

            p.setPen(m_theme.label);
            p.drawText(QPointF(pt.x() + 7.0, pt.y() + 4.0), ap.name);
        }
    }
}

void MapWidget::drawRangeRings(QPainter &p)
{
    if (!m_settings || m_settings->rangeRings == RangeRingMode::Off)
        return;

    // Mỗi chế độ gồm bước "đậm" và bước "mảnh" (0 nghĩa là không có vòng mảnh)
    double majorStep = 0.0;
    double minorStep = 0.0;
    switch (m_settings->rangeRings) {
    case RangeRingMode::R5km:  majorStep = 5.0;  minorStep = 0.0; break;
    case RangeRingMode::R1km:  majorStep = 5.0;  minorStep = 1.0; break;
    case RangeRingMode::R05km: majorStep = 1.0;  minorStep = 0.5; break;
    case RangeRingMode::R01km: majorStep = 0.5;  minorStep = 0.1; break;
    case RangeRingMode::Off:   return;
    }

    const double maxKm = m_settings->maxRangeKm;
    if (maxKm <= 0.0 || majorStep <= 0.0)
        return;

    const QPointF c = geoToScreen(m_settings->radarLat, m_settings->radarLng);

    // Màu lưới lấy theo màu đường quét nhưng mờ đi, để lưới không lấn át mục tiêu
    QColor majorColor = m_settings->colorSweep;
    majorColor.setAlpha(110);
    QColor minorColor = majorColor;
    minorColor.setAlpha(55);

    p.setBrush(Qt::NoBrush);

    /// Vẽ một loạt vòng tròn cách đều nhau; bỏ qua vòng trùng với bước đậm.
    auto drawRings = [&](double stepKm, const QColor &color, double penWidth, bool skipMajor) {
        if (stepKm <= 0.0)
            return;
        // Bỏ qua nếu các vòng quá sát nhau trên màn hình
        if (kmToPixelsY(stepKm) < kMinRingSpacingPx)
            return;
        QPen pen(color);
        pen.setWidthF(penWidth);
        p.setPen(pen);

        const int count = static_cast<int>(std::floor(maxKm / stepKm + 1e-9));
        for (int i = 1; i <= count; ++i) {
            const double km = i * stepKm;
            if (skipMajor) {
                // Vòng này đã được vẽ ở lượt "đậm" rồi thì không vẽ lại
                const double ratio = km / majorStep;
                if (std::abs(ratio - std::round(ratio)) < 1e-6)
                    continue;
            }
            p.drawEllipse(c, kmToPixelsX(km), kmToPixelsY(km));
        }
    };

    drawRings(minorStep, minorColor, 1.0, true);
    drawRings(majorStep, majorColor, 1.4, false);
}

void MapWidget::drawBearingLines(QPainter &p)
{
    if (!m_settings || m_settings->bearingLines == BearingLineMode::Off)
        return;

    double majorStep = 0.0;
    double minorStep = 0.0;
    switch (m_settings->bearingLines) {
    case BearingLineMode::D30: majorStep = 30.0; minorStep = 0.0;  break;
    case BearingLineMode::D10: majorStep = 30.0; minorStep = 10.0; break;
    case BearingLineMode::D5:  majorStep = 10.0; minorStep = 5.0;  break;
    case BearingLineMode::Off: return;
    }

    const double maxKm = m_settings->maxRangeKm;
    if (maxKm <= 0.0 || majorStep <= 0.0)
        return;

    const QPointF c = geoToScreen(m_settings->radarLat, m_settings->radarLng);
    const double rx = kmToPixelsX(maxKm);
    const double ry = kmToPixelsY(maxKm);

    QColor majorColor = m_settings->colorSweep;
    majorColor.setAlpha(110);
    QColor minorColor = majorColor;
    minorColor.setAlpha(55);

    /// Vẽ các đường chia độ cách đều nhau, từ tâm đài ra đến vòng cự ly tối đa.
    auto drawSpokes = [&](double stepDeg, const QColor &color, double penWidth, bool skipMajor) {
        if (stepDeg <= 0.0)
            return;
        QPen pen(color);
        pen.setWidthF(penWidth);
        p.setPen(pen);

        for (double a = 0.0; a < 360.0 - 1e-9; a += stepDeg) {
            if (skipMajor) {
                const double ratio = a / majorStep;
                if (std::abs(ratio - std::round(ratio)) < 1e-6)
                    continue;
            }
            const double r = deg2rad(a);
            // Điểm cuối nằm đúng trên elip cự ly tối đa
            p.drawLine(c, c + QPointF(rx * std::sin(r), -ry * std::cos(r)));
        }
    };

    drawSpokes(minorStep, minorColor, 1.0, true);
    drawSpokes(majorStep, majorColor, 1.4, false);
}

void MapWidget::drawSweep(QPainter &p)
{
    if (!m_settings)
        return;

    const QPointF c = geoToScreen(m_settings->radarLat, m_settings->radarLng);
    const double maxKm = m_settings->maxRangeKm;
    const double rx = kmToPixelsX(maxKm);
    const double ry = kmToPixelsY(maxKm);

    const double a = deg2rad(m_sweepAzimuth);
    const QPointF tip = c + QPointF(rx * std::sin(a), -ry * std::cos(a));

    const QColor sweep = m_settings->colorSweep;

    // Nét mờ rộng bên dưới tạo cảm giác chùm tia, nét mảnh sắc nét bên trên
    QColor glow = sweep;
    glow.setAlpha(45);
    QPen glowPen(glow);
    glowPen.setWidthF(5.0);
    glowPen.setCapStyle(Qt::RoundCap);
    p.setPen(glowPen);
    p.drawLine(c, tip);

    QPen linePen(sweep);
    linePen.setWidthF(1.6);
    p.setPen(linePen);
    p.drawLine(c, tip);

    // Ký hiệu tâm đài: dấu cộng nhỏ, để vị trí đài luôn nhìn thấy được
    // kể cả khi đã tắt hết vòng tròn cự ly và đường chia độ.
    QPen centerPen(sweep);
    centerPen.setWidthF(1.4);
    p.setPen(centerPen);
    p.drawLine(c + QPointF(-6.0, 0.0), c + QPointF(6.0, 0.0));
    p.drawLine(c + QPointF(0.0, -6.0), c + QPointF(0.0, 6.0));
}

void MapWidget::drawNoDataHint(QPainter &p)
{
    QFont f;
    f.setFamilies(Theme::fontFamilies());
    f.setPointSize(11);
    p.setFont(f);
    p.setPen(Theme::kTextDim);

    const QString text =
        tr("Chưa nạp được nền bản đồ số.\n"
           "Hãy copy thư mục MapChuan vào cùng chỗ với file chạy rồi mở lại phần mềm.");
    p.drawText(rect().adjusted(20, 20, -20, -20), Qt::AlignCenter | Qt::TextWordWrap, text);
}

// ---------------------------------------------------------- Tương tác chuột

void MapWidget::mousePressEvent(QMouseEvent *event)
{
    if (event->button() != Qt::LeftButton) {
        QWidget::mousePressEvent(event);
        return;
    }

    m_mouseDown = true;
    m_dragging = false;
    m_dragStart = event->position();
    m_dragCenterLat = m_centerLat;
    m_dragCenterLng = m_centerLng;

    // Đổi con trỏ thành bàn tay nắm giữ trong suốt thời gian giữ chuột trái
    setCursor(Qt::ClosedHandCursor);
    setFocus();
}

void MapWidget::mouseMoveEvent(QMouseEvent *event)
{
    const QPointF pos = event->position();

    double lat = 0.0, lng = 0.0;
    screenToGeo(pos, &lat, &lng);
    emit mouseGeoMoved(lat, lng);

    if (m_mouseDown) {
        const double dx = pos.x() - m_dragStart.x();
        const double dy = pos.y() - m_dragStart.y();
        // Chỉ coi là kéo khi đã đi quá 4 pixel, tránh rung tay khi bấm chọn
        if (!m_dragging && (std::abs(dx) > 4.0 || std::abs(dy) > 4.0))
            m_dragging = true;
        if (m_dragging) {
            m_centerLng = m_dragCenterLng - dx / m_scale;
            m_centerLat = m_dragCenterLat + dy / m_scale;
            m_userChangedView = true;
            update();
        }
    }
}

void MapWidget::mouseReleaseEvent(QMouseEvent *event)
{
    if (event->button() != Qt::LeftButton) {
        QWidget::mouseReleaseEvent(event);
        return;
    }
    m_mouseDown = false;
    m_dragging = false;
    setCursor(Qt::CrossCursor);
}

void MapWidget::wheelEvent(QWheelEvent *event)
{
    const int delta = event->angleDelta().y();
    if (delta == 0)
        return;
    zoomAt(event->position(), delta > 0 ? 1.25 : 1.0 / 1.25);
    event->accept();
}

void MapWidget::leaveEvent(QEvent *event)
{
    emit mouseGeoLeft();
    QWidget::leaveEvent(event);
}

void MapWidget::zoomAt(const QPointF &anchor, double factor)
{
    double lat = 0.0, lng = 0.0;
    screenToGeo(anchor, &lat, &lng);

    const double newScale = qBound(kMinScale, m_scale * factor, kMaxScale);
    if (qFuzzyCompare(newScale, m_scale))
        return;
    m_scale = newScale;
    m_userChangedView = true;

    // Giữ nguyên điểm địa lý nằm dưới con trỏ sau khi đổi tỉ lệ
    m_centerLng = lng - (anchor.x() - width() / 2.0) / m_scale;
    m_centerLat = lat + (anchor.y() - height() / 2.0) / m_scale;

    syncZoomSlider();
    update();
}

// ---------------------------------------------------------- Thanh trượt zoom

void MapWidget::buildZoomBar()
{
    m_zoomBar = new QFrame(this);
    m_zoomBar->setObjectName(QStringLiteral("ZoomBar"));
    m_zoomBar->setStyleSheet(QStringLiteral(R"(
QFrame#ZoomBar {
    background-color: rgba(22, 24, 26, 190);
    border: 1px solid #3A4147;
}
QFrame#ZoomBar QToolButton {
    background-color: transparent;
    color: #C6CCD2;
    border: none;
    font-size: 15px;
    font-weight: bold;
    min-width: 20px;
    min-height: 20px;
}
QFrame#ZoomBar QToolButton:hover {
    color: #4FB0D8;
}
)"));

    m_zoomOutBtn = new QToolButton(m_zoomBar);
    m_zoomOutBtn->setText(QStringLiteral("−")); // dấu trừ
    m_zoomOutBtn->setToolTip(tr("Thu nhỏ"));
    m_zoomOutBtn->setCursor(Qt::ArrowCursor);

    m_zoomInBtn = new QToolButton(m_zoomBar);
    m_zoomInBtn->setText(QStringLiteral("+"));
    m_zoomInBtn->setToolTip(tr("Phóng to"));
    m_zoomInBtn->setCursor(Qt::ArrowCursor);

    m_zoomSlider = new QSlider(Qt::Horizontal, m_zoomBar);
    m_zoomSlider->setRange(0, 100);
    m_zoomSlider->setFixedWidth(150);
    m_zoomSlider->setToolTip(tr("Mức phóng bản đồ"));
    m_zoomSlider->setCursor(Qt::ArrowCursor);

    auto *lay = new QHBoxLayout(m_zoomBar);
    lay->setContentsMargins(6, 3, 6, 3);
    lay->setSpacing(6);
    lay->addWidget(m_zoomOutBtn);
    lay->addWidget(m_zoomSlider);
    lay->addWidget(m_zoomInBtn);

    connect(m_zoomSlider, &QSlider::valueChanged, this, [this](int value) {
        const double newScale = zoomLevelToScale(value);
        if (qFuzzyCompare(newScale, m_scale))
            return;
        // Kéo thanh trượt thì phóng quanh tâm panel
        m_scale = newScale;
        m_userChangedView = true;
        update();
    });
    connect(m_zoomOutBtn, &QToolButton::clicked, this, [this] {
        zoomAt(QPointF(width() / 2.0, height() / 2.0), 1.0 / 1.25);
    });
    connect(m_zoomInBtn, &QToolButton::clicked, this, [this] {
        zoomAt(QPointF(width() / 2.0, height() / 2.0), 1.25);
    });

    syncZoomSlider();
}

void MapWidget::layoutZoomBar()
{
    if (!m_zoomBar)
        return;
    const QSize hint = m_zoomBar->sizeHint();
    const int margin = 14;
    m_zoomBar->setGeometry(width() - hint.width() - margin,
                           height() - hint.height() - margin,
                           hint.width(), hint.height());
}

void MapWidget::syncZoomSlider()
{
    if (!m_zoomSlider)
        return;
    // Chặn tín hiệu để việc đồng bộ ngược không kích hoạt lại valueChanged
    const QSignalBlocker blocker(m_zoomSlider);
    m_zoomSlider->setValue(static_cast<int>(std::lround(scaleToZoomLevel(m_scale))));
}

double MapWidget::scaleToZoomLevel(double scale) const
{
    // Thang logarit: mỗi bước trên thanh trượt đổi tỉ lệ theo cùng một hệ số nhân
    return std::log(scale / kMinScale) / std::log(kMaxScale / kMinScale) * 100.0;
}

double MapWidget::zoomLevelToScale(double level) const
{
    return kMinScale * std::pow(kMaxScale / kMinScale, qBound(0.0, level, 100.0) / 100.0);
}

void MapWidget::resizeEvent(QResizeEvent *event)
{
    QWidget::resizeEvent(event);
    layoutZoomBar();

    // Mức phóng ban đầu tính theo chiều cao panel, mà chiều cao đó chỉ đúng sau khi
    // cửa sổ đã vào chế độ toàn màn hình — trên Linux việc này xảy ra muộn hơn lúc
    // chương trình hiện ra. Vì vậy cứ canh lại khung nhìn theo tâm đài cho tới khi
    // người dùng tự kéo hoặc phóng bản đồ lần đầu.
    if (!m_userChangedView)
        centerOnRadar();
}
