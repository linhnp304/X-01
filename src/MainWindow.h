#pragma once

#include "AppSettings.h"
#include "map/MapData.h"

#include <QMainWindow>

class AmplitudeWidget;
class ColorsTab;
class ConnectionTab;
class GeneralSettingsTab;
class ListTab;
class MapWidget;

class QLabel;
class QTabWidget;
class QTimer;

/// Cửa sổ chính — ghép 3 panel theo đúng bố cục yêu cầu:
///
///   +-------------------------------------------+--------------------+
///   |                                           |  Panel 2.1 (75%)   |
///   |         Panel 1 — bản đồ (70%)            |  các tab điều khiển|
///   |                                           +--------------------+
///   |                                           |  Panel 2.2 (25%)   |
///   |                                           |  cửa sổ biên độ    |
///   +-------------------------------------------+--------------------+
///   |            Panel 3 — thanh trạng thái                          |
///   +----------------------------------------------------------------+
class MainWindow : public QMainWindow
{
    Q_OBJECT

public:
    explicit MainWindow(QWidget *parent = nullptr);
    ~MainWindow() override;

protected:
    void keyPressEvent(QKeyEvent *event) override;
    void showEvent(QShowEvent *event) override;

private:
    void buildUi();
    void buildStatusBar();
    void connectSignals();

    /// Áp dụng cấu hình lên bản đồ rồi ghi ra tệp .json.
    void applyAndSaveSettings();

    /// Cập nhật ô toạ độ tâm đài trên thanh trạng thái.
    void updateRadarCenterLabel();

    /// Định dạng một cặp toạ độ, làm tròn đến 0,000001 độ.
    static QString formatLatLng(double lat, double lng);

    AppSettings m_settings;
    MapData m_mapData;

    MapWidget *m_map = nullptr;
    QTabWidget *m_tabs = nullptr;
    AmplitudeWidget *m_amplitude = nullptr;

    ListTab *m_listTab = nullptr;
    ConnectionTab *m_connectionTab = nullptr;
    ColorsTab *m_colorsTab = nullptr;
    GeneralSettingsTab *m_generalTab = nullptr;

    // Thanh trạng thái
    QLabel *m_statusLeft = nullptr;   ///< trạng thái kết nối đến các máy khác
    QLabel *m_statusCenter = nullptr; ///< thời gian hệ thống + toạ độ con trỏ
    QLabel *m_statusRight = nullptr;  ///< toạ độ tâm đài

    /// Làm tươi ô giữa thanh trạng thái 10 lần/giây (đồng hồ + toạ độ con trỏ).
    QTimer *m_statusTimer = nullptr;

    // Vị trí con trỏ trên bản đồ. Chỉ lưu số ở đây, việc ghép chuỗi để cho bộ đếm
    // thời gian làm — mỗi lần rê chuột không phải dựng lại chuỗi và vẽ lại nhãn.
    double m_mouseLat = 0.0;
    double m_mouseLng = 0.0;
    bool m_mouseOnMap = false;

    bool m_firstShow = true;
};
