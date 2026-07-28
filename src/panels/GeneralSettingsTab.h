#pragma once

#include <QWidget>

class AppSettings;

class QButtonGroup;
class QCheckBox;
class QDoubleSpinBox;
class QLabel;
class QPushButton;
class QSlider;
class QSpinBox;

/// Tab "Cài đặt chung" — tab mở mặc định khi chạy phần mềm.
///
/// Tab thao tác trực tiếp trên đối tượng AppSettings do MainWindow sở hữu,
/// sau mỗi thay đổi thì phát tín hiệu settingsChanged() để MainWindow vẽ lại
/// bản đồ và ghi cấu hình ra tệp .json.
class GeneralSettingsTab : public QWidget
{
    Q_OBJECT

public:
    explicit GeneralSettingsTab(AppSettings *settings, QWidget *parent = nullptr);

    /// Nạp giá trị từ AppSettings lên các điều khiển (gọi một lần lúc khởi động).
    void loadFromSettings();

signals:
    /// Một cài đặt bất kỳ vừa thay đổi.
    void settingsChanged();
    /// Người dùng bấm nút "Áp dụng" của ô toạ độ tâm đài.
    void radarCenterApplied();
    /// Người dùng đã xác nhận muốn thoát chương trình.
    void exitRequested();

private:
    void buildUi();

    AppSettings *m_settings = nullptr;

    // Độ sáng bản đồ
    QSlider *m_brightnessSlider = nullptr;
    QLabel *m_brightnessValue = nullptr;

    // Ẩn/hiện các lớp bản đồ
    QCheckBox *m_chkAirRoutes = nullptr;
    QCheckBox *m_chkAirports = nullptr;
    QCheckBox *m_chkPlaceNames = nullptr;
    QCheckBox *m_chkProvinces = nullptr;

    // Toạ độ tâm đài
    QDoubleSpinBox *m_radarLat = nullptr;
    QDoubleSpinBox *m_radarLng = nullptr;
    QPushButton *m_applyRadarBtn = nullptr;

    // Vòng tròn cự ly và đường chia độ
    QDoubleSpinBox *m_maxRange = nullptr;
    QButtonGroup *m_ringGroup = nullptr;
    QButtonGroup *m_bearingGroup = nullptr;

    // Quỹ đạo
    QCheckBox *m_chkTrackInfo = nullptr;
    QCheckBox *m_chkAutoDelete = nullptr;
    QSpinBox *m_autoDeleteSeconds = nullptr;

    // Thoát chương trình
    QPushButton *m_exitBtn = nullptr;
};
