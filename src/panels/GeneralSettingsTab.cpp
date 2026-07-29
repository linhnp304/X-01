#include "panels/GeneralSettingsTab.h"

#include "AppSettings.h"

#include <QAbstractButton>
#include <QButtonGroup>
#include <QCheckBox>
#include <QDoubleSpinBox>
#include <QFormLayout>
#include <QFrame>
#include <QHBoxLayout>
#include <QLabel>
#include <QMessageBox>
#include <QPushButton>
#include <QRadioButton>
#include <QSignalBlocker>
#include <QSlider>
#include <QSpinBox>
#include <QStringList>
#include <QVBoxLayout>
#include <QVector>

namespace {

/// Tiêu đề in đậm màu nhấn, mở đầu mỗi nhóm cài đặt.
QLabel *sectionTitle(const QString &text)
{
    auto *l = new QLabel(text);
    l->setObjectName(QStringLiteral("SectionTitle"));
    return l;
}

/// Đường kẻ mảnh ngăn cách các nhóm cài đặt.
QFrame *separator()
{
    auto *f = new QFrame;
    f->setObjectName(QStringLiteral("Separator"));
    f->setFrameShape(QFrame::HLine);
    f->setFixedHeight(1);
    return f;
}

/// Hàng radio button nằm ngang, kèm nhãn mô tả bên trái.
/// labels/ids phải cùng số phần tử; ids là giá trị enum ép về int.
QWidget *radioRow(const QString &caption,
                  const QStringList &labels,
                  const QVector<int> &ids,
                  QButtonGroup *group)
{
    auto *row = new QWidget;
    auto *lay = new QHBoxLayout(row);
    lay->setContentsMargins(0, 0, 0, 0);
    // Khoảng cách giữa các nút chọn: đủ rộng để mắt tách được từng lựa chọn,
    // nhưng vẫn xếp vừa một hàng trong panel rộng 30% màn hình.
    lay->setSpacing(14);

    auto *cap = new QLabel(caption, row);
    cap->setMinimumWidth(112);
    lay->addWidget(cap);

    for (int i = 0; i < labels.size() && i < ids.size(); ++i) {
        auto *rb = new QRadioButton(labels.at(i), row);
        group->addButton(rb, ids.at(i));
        lay->addWidget(rb);
    }
    lay->addStretch(1);
    return row;
}

} // namespace

GeneralSettingsTab::GeneralSettingsTab(AppSettings *settings, QWidget *parent)
    : QWidget(parent)
    , m_settings(settings)
{
    buildUi();
    loadFromSettings();
}

void GeneralSettingsTab::buildUi()
{
    auto *root = new QVBoxLayout(this);
    root->setContentsMargins(12, 12, 12, 12);
    root->setSpacing(8);

    // ---------------- Độ sáng bản đồ ----------------
    root->addWidget(sectionTitle(tr("Độ sáng bản đồ")));

    auto *brightRow = new QWidget(this);
    auto *brightLay = new QHBoxLayout(brightRow);
    brightLay->setContentsMargins(0, 0, 0, 0);
    brightLay->setSpacing(8);

    m_brightnessSlider = new QSlider(Qt::Horizontal, brightRow);
    m_brightnessSlider->setRange(1, 10);
    m_brightnessSlider->setPageStep(1);
    m_brightnessSlider->setTickPosition(QSlider::TicksBelow);
    m_brightnessSlider->setTickInterval(1);

    m_brightnessValue = new QLabel(brightRow);
    m_brightnessValue->setMinimumWidth(20);
    m_brightnessValue->setAlignment(Qt::AlignCenter);

    auto *lblMin = new QLabel(QStringLiteral("1"), brightRow);
    lblMin->setObjectName(QStringLiteral("Dim"));
    auto *lblMax = new QLabel(QStringLiteral("10"), brightRow);
    lblMax->setObjectName(QStringLiteral("Dim"));

    brightLay->addWidget(lblMin);
    brightLay->addWidget(m_brightnessSlider, 1);
    brightLay->addWidget(lblMax);
    brightLay->addWidget(m_brightnessValue);
    root->addWidget(brightRow);

    connect(m_brightnessSlider, &QSlider::valueChanged, this, [this](int v) {
        m_settings->mapBrightness = v;
        m_brightnessValue->setText(QString::number(v));
        emit settingsChanged();
    });

    // ---------------- Ẩn/Hiện các lớp bản đồ ----------------
    root->addSpacing(4);
    root->addWidget(sectionTitle(tr("Ẩn/Hiện các lớp bản đồ")));

    m_chkAirRoutes = new QCheckBox(tr("Đường bay dân dụng"), this);
    m_chkAirports = new QCheckBox(tr("Sân bay"), this);
    m_chkRivers = new QCheckBox(tr("Sông ngòi"), this);
    m_chkPlaceNames = new QCheckBox(tr("Tên địa danh"), this);
    m_chkProvinces = new QCheckBox(tr("Địa phận tỉnh/thành phố"), this);

    for (QCheckBox *c : { m_chkAirRoutes, m_chkAirports, m_chkRivers,
                          m_chkPlaceNames, m_chkProvinces })
        root->addWidget(c);

    connect(m_chkAirRoutes, &QCheckBox::toggled, this, [this](bool on) {
        m_settings->showAirRoutes = on;
        emit settingsChanged();
    });
    connect(m_chkAirports, &QCheckBox::toggled, this, [this](bool on) {
        m_settings->showAirports = on;
        emit settingsChanged();
    });
    connect(m_chkRivers, &QCheckBox::toggled, this, [this](bool on) {
        m_settings->showRivers = on;
        emit settingsChanged();
    });
    connect(m_chkPlaceNames, &QCheckBox::toggled, this, [this](bool on) {
        m_settings->showPlaceNames = on;
        emit settingsChanged();
    });
    connect(m_chkProvinces, &QCheckBox::toggled, this, [this](bool on) {
        m_settings->showProvinces = on;
        emit settingsChanged();
    });

    // ---------------- Toạ độ tâm đài ----------------
    root->addSpacing(6);
    root->addWidget(separator());
    root->addWidget(sectionTitle(tr("Toạ độ tâm đài")));

    m_radarLat = new QDoubleSpinBox(this);
    m_radarLat->setDecimals(6); // làm tròn đến 0,000001 độ
    m_radarLat->setRange(-90.0, 90.0);
    m_radarLat->setSingleStep(0.000001);
    m_radarLat->setKeyboardTracking(false);

    m_radarLng = new QDoubleSpinBox(this);
    m_radarLng->setDecimals(6);
    m_radarLng->setRange(-180.0, 180.0);
    m_radarLng->setSingleStep(0.000001);
    m_radarLng->setKeyboardTracking(false);

    auto *radarForm = new QFormLayout;
    radarForm->setContentsMargins(0, 0, 0, 0);
    radarForm->setLabelAlignment(Qt::AlignLeft | Qt::AlignVCenter);
    radarForm->setSpacing(6);
    radarForm->addRow(tr("Vĩ độ (lat):"), m_radarLat);
    radarForm->addRow(tr("Kinh độ (lng):"), m_radarLng);
    root->addLayout(radarForm);

    m_applyRadarBtn = new QPushButton(tr("Áp dụng"), this);
    auto *applyRow = new QHBoxLayout;
    applyRow->setContentsMargins(0, 0, 0, 0);
    applyRow->addStretch(1);
    applyRow->addWidget(m_applyRadarBtn);
    root->addLayout(applyRow);

    // Toạ độ tâm đài chỉ có hiệu lực khi bấm "Áp dụng", không đổi theo từng lần gõ phím
    connect(m_applyRadarBtn, &QPushButton::clicked, this, [this] {
        m_settings->radarLat = m_radarLat->value();
        m_settings->radarLng = m_radarLng->value();
        emit radarCenterApplied();
        emit settingsChanged();
    });

    // ---------------- Vòng tròn cự ly và đường chia độ ----------------
    root->addSpacing(6);
    root->addWidget(separator());
    root->addWidget(sectionTitle(tr("Vòng tròn cự ly và đường chia độ")));

    m_maxRange = new QDoubleSpinBox(this);
    m_maxRange->setDecimals(1);
    m_maxRange->setRange(1.0, 2000.0);
    m_maxRange->setSingleStep(5.0);
    m_maxRange->setKeyboardTracking(false);

    auto *rangeForm = new QFormLayout;
    rangeForm->setContentsMargins(0, 0, 0, 0);
    rangeForm->setLabelAlignment(Qt::AlignLeft | Qt::AlignVCenter);
    rangeForm->setSpacing(6);
    rangeForm->addRow(tr("Cự ly tối đa (km):"), m_maxRange);
    root->addLayout(rangeForm);

    connect(m_maxRange, &QDoubleSpinBox::valueChanged, this, [this](double v) {
        m_settings->maxRangeKm = v;
        emit settingsChanged();
    });

    m_ringGroup = new QButtonGroup(this);
    root->addWidget(radioRow(tr("Vòng tròn cự ly:"),
                             { tr("5km"), tr("1km"), tr("0.5km"), tr("0.1km"), tr("Tắt") },
                             { static_cast<int>(RangeRingMode::R5km),
                               static_cast<int>(RangeRingMode::R1km),
                               static_cast<int>(RangeRingMode::R05km),
                               static_cast<int>(RangeRingMode::R01km),
                               static_cast<int>(RangeRingMode::Off) },
                             m_ringGroup));

    m_bearingGroup = new QButtonGroup(this);
    root->addWidget(radioRow(tr("Đường chia độ:"),
                             { tr("30 độ"), tr("10 độ"), tr("5 độ"), tr("Tắt") },
                             { static_cast<int>(BearingLineMode::D30),
                               static_cast<int>(BearingLineMode::D10),
                               static_cast<int>(BearingLineMode::D5),
                               static_cast<int>(BearingLineMode::Off) },
                             m_bearingGroup));

    connect(m_ringGroup, &QButtonGroup::idToggled, this, [this](int id, bool checked) {
        if (!checked)
            return;
        m_settings->rangeRings = static_cast<RangeRingMode>(id);
        emit settingsChanged();
    });
    connect(m_bearingGroup, &QButtonGroup::idToggled, this, [this](int id, bool checked) {
        if (!checked)
            return;
        m_settings->bearingLines = static_cast<BearingLineMode>(id);
        emit settingsChanged();
    });

    // ---------------- Quỹ đạo ----------------
    root->addSpacing(6);
    root->addWidget(separator());

    m_chkTrackInfo = new QCheckBox(tr("Hiện thông tin quỹ đạo"), this);
    root->addWidget(m_chkTrackInfo);
    connect(m_chkTrackInfo, &QCheckBox::toggled, this, [this](bool on) {
        m_settings->showTrackInfo = on;
        emit settingsChanged();
    });

    m_chkAutoDelete = new QCheckBox(tr("Tự động xoá quỹ đạo khi mất cập nhật"), this);
    root->addWidget(m_chkAutoDelete);

    m_autoDeleteSeconds = new QSpinBox(this);
    m_autoDeleteSeconds->setRange(1, 3600);
    m_autoDeleteSeconds->setFixedWidth(80);
    m_autoDeleteSeconds->setKeyboardTracking(false);

    auto *autoRow = new QWidget(this);
    auto *autoLay = new QHBoxLayout(autoRow);
    autoLay->setContentsMargins(22, 0, 0, 0); // thụt vào cho thấy phụ thuộc ô tick ở trên
    autoLay->setSpacing(6);
    autoLay->addWidget(new QLabel(tr("Sau"), autoRow));
    autoLay->addWidget(m_autoDeleteSeconds);
    autoLay->addWidget(new QLabel(tr("giây không nhận được cập nhật mới"), autoRow));
    autoLay->addStretch(1);
    root->addWidget(autoRow);

    connect(m_chkAutoDelete, &QCheckBox::toggled, this, [this, autoRow](bool on) {
        m_settings->autoDeleteTrack = on;
        autoRow->setEnabled(on);
        emit settingsChanged();
    });
    connect(m_autoDeleteSeconds, &QSpinBox::valueChanged, this, [this](int v) {
        m_settings->autoDeleteSeconds = v;
        emit settingsChanged();
    });

    // ---------------- Thoát chương trình ----------------
    // Đẩy nút xuống đáy tab: phần co giãn nằm TRƯỚC nên mọi nhóm cài đặt ở trên
    // giữ nguyên chiều cao, chỗ trống dồn hết vào giữa.
    root->addStretch(1);
    root->addWidget(separator());

    m_exitBtn = new QPushButton(tr("Thoát chương trình"), this);
    m_exitBtn->setObjectName(QStringLiteral("ExitButton"));
    m_exitBtn->setMinimumHeight(30);
    root->addWidget(m_exitBtn);

    connect(m_exitBtn, &QPushButton::clicked, this, [this] {
        // Phần mềm chạy toàn màn hình nên bấm nhầm là mất luôn tình huống đang theo dõi.
        // Hỏi lại một lần, mặc định chọn "Không" cho an toàn.
        QMessageBox box(this);
        box.setIcon(QMessageBox::Question);
        box.setWindowTitle(tr("Thoát chương trình"));
        box.setText(tr("Thoát phần mềm X-01?"));
        QPushButton *yes = box.addButton(tr("Thoát"), QMessageBox::AcceptRole);
        QPushButton *no = box.addButton(tr("Không"), QMessageBox::RejectRole);
        box.setDefaultButton(no);
        box.exec();
        if (box.clickedButton() == yes)
            emit exitRequested();
    });
}

void GeneralSettingsTab::loadFromSettings()
{
    // Chặn tín hiệu trong lúc nạp để không ghi đè cấu hình bằng chính giá trị vừa đọc
    const QSignalBlocker b1(m_brightnessSlider);
    const QSignalBlocker b2(m_chkAirRoutes);
    const QSignalBlocker b3(m_chkAirports);
    const QSignalBlocker b3b(m_chkRivers);
    const QSignalBlocker b4(m_chkPlaceNames);
    const QSignalBlocker b5(m_chkProvinces);
    const QSignalBlocker b6(m_radarLat);
    const QSignalBlocker b7(m_radarLng);
    const QSignalBlocker b8(m_maxRange);
    const QSignalBlocker b9(m_ringGroup);
    const QSignalBlocker b10(m_bearingGroup);
    const QSignalBlocker b11(m_chkTrackInfo);
    const QSignalBlocker b12(m_chkAutoDelete);
    const QSignalBlocker b13(m_autoDeleteSeconds);

    m_brightnessSlider->setValue(m_settings->mapBrightness);
    m_brightnessValue->setText(QString::number(m_settings->mapBrightness));

    m_chkAirRoutes->setChecked(m_settings->showAirRoutes);
    m_chkAirports->setChecked(m_settings->showAirports);
    m_chkRivers->setChecked(m_settings->showRivers);
    m_chkPlaceNames->setChecked(m_settings->showPlaceNames);
    m_chkProvinces->setChecked(m_settings->showProvinces);

    m_radarLat->setValue(m_settings->radarLat);
    m_radarLng->setValue(m_settings->radarLng);

    m_maxRange->setValue(m_settings->maxRangeKm);

    if (QAbstractButton *b = m_ringGroup->button(static_cast<int>(m_settings->rangeRings)))
        b->setChecked(true);
    if (QAbstractButton *b = m_bearingGroup->button(static_cast<int>(m_settings->bearingLines)))
        b->setChecked(true);

    m_chkTrackInfo->setChecked(m_settings->showTrackInfo);
    m_chkAutoDelete->setChecked(m_settings->autoDeleteTrack);
    m_autoDeleteSeconds->setValue(m_settings->autoDeleteSeconds);

    // Ô nhập số giây chỉ dùng được khi đã bật tự động xoá
    if (QWidget *row = m_autoDeleteSeconds->parentWidget())
        row->setEnabled(m_settings->autoDeleteTrack);
}
