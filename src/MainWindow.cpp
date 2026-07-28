#include "MainWindow.h"

#include "Theme.h"
#include "map/MapWidget.h"
#include "panels/AmplitudeWidget.h"
#include "panels/ColorsTab.h"
#include "panels/ConnectionTab.h"
#include "panels/GeneralSettingsTab.h"
#include "panels/ListTab.h"

#include <QCoreApplication>
#include <QDateTime>
#include <QFrame>
#include <QHBoxLayout>
#include <QKeyEvent>
#include <QLabel>
#include <QScrollArea>
#include <QSplitter>
#include <QStatusBar>
#include <QTabWidget>
#include <QTimer>
#include <QVBoxLayout>

namespace {

/// Tỉ lệ chia panel — dùng "stretch factor" của QSplitter nên tỉ lệ được giữ
/// đúng ở mọi độ phân giải, không riêng 1920x1080.
constexpr int kPanel1Percent = 70;  // bản đồ
constexpr int kPanel2Percent = 30;  // cột điều khiển bên phải
constexpr int kPanel21Percent = 75; // các tab điều khiển
constexpr int kPanel22Percent = 25; // cửa sổ biên độ

/// Bọc một tab vào vùng cuộn dọc, để nội dung dài vẫn xem được khi panel hẹp.
QScrollArea *wrapInScroll(QWidget *content)
{
    auto *area = new QScrollArea;
    area->setWidget(content);
    area->setWidgetResizable(true);
    area->setHorizontalScrollBarPolicy(Qt::ScrollBarAlwaysOff);
    area->setFrameShape(QFrame::NoFrame);
    return area;
}

} // namespace

MainWindow::MainWindow(QWidget *parent)
    : QMainWindow(parent)
{
    setWindowTitle(tr("X-01 — Màn hình trắc thủ ra đa"));

    // Đọc cấu hình trước khi dựng giao diện để các điều khiển hiện đúng giá trị đã lưu
    m_settings.load();

    // Nạp nền bản đồ số từ thư mục MapChuan nằm cùng chỗ với file chạy
    m_mapData.load(QCoreApplication::applicationDirPath() + QStringLiteral("/MapChuan"));

    buildUi();
    buildStatusBar();
    connectSignals();

    m_map->setSettings(&m_settings);
    m_map->setMapData(&m_mapData);
    updateRadarCenterLabel();
}

MainWindow::~MainWindow()
{
    // MapWidget và các tab đang giữ con trỏ tới m_settings và m_mapData.
    // Các thành viên này bị huỷ TRƯỚC khi ~QMainWindow xoá cây widget con,
    // nên phải chủ động xoá cây widget ngay tại đây cho đúng thứ tự.
    delete takeCentralWidget();
}

void MainWindow::buildUi()
{
    // ---- Panel 1: bản đồ ----
    m_map = new MapWidget(this);

    // ---- Panel 2.1: các tab điều khiển ----
    m_tabs = new QTabWidget(this);
    m_tabs->setDocumentMode(true);

    m_listTab = new ListTab(m_tabs);
    m_connectionTab = new ConnectionTab(m_tabs);
    m_colorsTab = new ColorsTab(&m_settings, m_tabs);
    m_generalTab = new GeneralSettingsTab(&m_settings, m_tabs);

    m_tabs->addTab(m_listTab, tr("Danh sách"));
    m_tabs->addTab(m_connectionTab, tr("Kết nối"));
    m_tabs->addTab(wrapInScroll(m_colorsTab), tr("Màu sắc"));
    m_tabs->addTab(wrapInScroll(m_generalTab), tr("Cài đặt chung"));

    // Mặc định mở tab "Cài đặt chung" khi chạy
    m_tabs->setCurrentIndex(m_tabs->count() - 1);

    // ---- Panel 2.2: cửa sổ biên độ ----
    m_amplitude = new AmplitudeWidget(this);

    // ---- Ghép panel 2.1 và 2.2 theo chiều dọc ----
    auto *rightSplitter = new QSplitter(Qt::Vertical, this);
    rightSplitter->addWidget(m_tabs);
    rightSplitter->addWidget(m_amplitude);
    rightSplitter->setStretchFactor(0, kPanel21Percent);
    rightSplitter->setStretchFactor(1, kPanel22Percent);
    rightSplitter->setChildrenCollapsible(false);
    rightSplitter->setHandleWidth(2);

    // ---- Ghép panel 1 và panel 2 theo chiều ngang ----
    auto *mainSplitter = new QSplitter(Qt::Horizontal, this);
    mainSplitter->addWidget(m_map);
    mainSplitter->addWidget(rightSplitter);
    mainSplitter->setStretchFactor(0, kPanel1Percent);
    mainSplitter->setStretchFactor(1, kPanel2Percent);
    mainSplitter->setChildrenCollapsible(false);
    mainSplitter->setHandleWidth(2);

    setCentralWidget(mainSplitter);

    // Ghi nhớ hai splitter để đặt tỉ lệ chính xác khi cửa sổ hiện lần đầu
    mainSplitter->setObjectName(QStringLiteral("MainSplitter"));
    rightSplitter->setObjectName(QStringLiteral("RightSplitter"));
}

void MainWindow::buildStatusBar()
{
    // Panel 3 — thanh trạng thái
    m_statusLeft = new QLabel(tr("Kết nối hệ thống: (sẽ làm ở bước sau)"), this);
    m_statusLeft->setObjectName(QStringLiteral("Placeholder"));

    m_statusCenter = new QLabel(this);
    m_statusCenter->setAlignment(Qt::AlignCenter);

    m_statusRight = new QLabel(this);
    m_statusRight->setAlignment(Qt::AlignRight | Qt::AlignVCenter);

    // addWidget: căn trái; addPermanentWidget: căn phải.
    // Ô giữa được cho stretch = 1 nên luôn nằm đúng giữa thanh.
    statusBar()->addWidget(m_statusLeft);
    statusBar()->addWidget(m_statusCenter, 1);
    statusBar()->addPermanentWidget(m_statusRight);
    statusBar()->setSizeGripEnabled(false);

    // Đồng hồ hệ thống, cập nhật mỗi giây
    m_clockTimer = new QTimer(this);
    m_clockTimer->setInterval(1000);
    connect(m_clockTimer, &QTimer::timeout, this, [this] {
        const QString now = QDateTime::currentDateTime().toString(QStringLiteral("dd/MM/yyyy  HH:mm:ss"));
        m_statusCenter->setText(m_mouseGeoText.isEmpty()
                                    ? now
                                    : now + QStringLiteral("     |     ") + m_mouseGeoText);
    });
    m_clockTimer->start();
}

void MainWindow::connectSignals()
{
    // --- Toạ độ con trỏ chuột trên bản đồ ---
    connect(m_map, &MapWidget::mouseGeoMoved, this, [this](double lat, double lng) {
        m_mouseGeoText = tr("Con trỏ: ") + formatLatLng(lat, lng);
    });
    connect(m_map, &MapWidget::mouseGeoLeft, this, [this] {
        m_mouseGeoText.clear();
    });

    // --- Cài đặt chung ---
    connect(m_generalTab, &GeneralSettingsTab::settingsChanged,
            this, &MainWindow::applyAndSaveSettings);
    connect(m_generalTab, &GeneralSettingsTab::radarCenterApplied, this, [this] {
        // Dịch chuyển tâm đài -> đưa luôn khung nhìn về vị trí mới cho dễ quan sát
        m_map->centerOnRadar();
        updateRadarCenterLabel();
    });

    // --- Màu sắc ---
    connect(m_colorsTab, &ColorsTab::settingsChanged,
            this, &MainWindow::applyAndSaveSettings);
}

void MainWindow::applyAndSaveSettings()
{
    m_map->applySettings();
    updateRadarCenterLabel();
    m_settings.save();
}

void MainWindow::updateRadarCenterLabel()
{
    m_statusRight->setText(tr("Tâm đài: ") + formatLatLng(m_settings.radarLat, m_settings.radarLng));
}

QString MainWindow::formatLatLng(double lat, double lng)
{
    // Làm tròn đến 0,000001 độ theo yêu cầu
    return QStringLiteral("%1, %2")
        .arg(lat, 0, 'f', 6)
        .arg(lng, 0, 'f', 6);
}

void MainWindow::showEvent(QShowEvent *event)
{
    QMainWindow::showEvent(event);

    if (!m_firstShow)
        return;
    m_firstShow = false;

    // Lúc này cửa sổ đã có kích thước thật -> chia panel đúng tỉ lệ 70/30 và 75/25.
    if (auto *mainSplitter = findChild<QSplitter *>(QStringLiteral("MainSplitter"))) {
        const int w = mainSplitter->width();
        mainSplitter->setSizes({ w * kPanel1Percent / 100, w * kPanel2Percent / 100 });
    }
    if (auto *rightSplitter = findChild<QSplitter *>(QStringLiteral("RightSplitter"))) {
        const int h = rightSplitter->height();
        rightSplitter->setSizes({ h * kPanel21Percent / 100, h * kPanel22Percent / 100 });
    }

    // Đưa khung nhìn về tâm đài. Hoãn một nhịp vòng lặp sự kiện để chờ splitter
    // bố trí lại xong, lúc đó MapWidget mới có chiều cao thật để tính mức phóng.
    QTimer::singleShot(0, this, [this] { m_map->centerOnRadar(); });
}

void MainWindow::keyPressEvent(QKeyEvent *event)
{
    // F11: bật/tắt toàn màn hình — tiện khi cần thao tác với cửa sổ khác lúc thử nghiệm
    if (event->key() == Qt::Key_F11) {
        if (isFullScreen())
            showNormal();
        else
            showFullScreen();
        event->accept();
        return;
    }
    QMainWindow::keyPressEvent(event);
}
