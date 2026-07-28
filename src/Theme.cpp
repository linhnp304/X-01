#include "Theme.h"

#include <QApplication>
#include <QFont>
#include <QPalette>
#include <QStringList>

QStringList Theme::fontFamilies()
{
    // Segoe UI có sẵn trên Windows; Noto Sans / DejaVu Sans có sẵn trên Ubuntu.
    // Cả ba đều đủ dấu tiếng Việt nên chữ hiển thị giống nhau ở hai nền tảng.
    return { QStringLiteral("Segoe UI"),
             QStringLiteral("Noto Sans"),
             QStringLiteral("DejaVu Sans"),
             QStringLiteral("Liberation Sans") };
}

void Theme::apply(QApplication &app)
{
    app.setStyle(QStringLiteral("Fusion"));

    QFont f;
    f.setFamilies(fontFamilies());
    f.setPointSize(10);
    app.setFont(f);

    // Bảng màu nền: đặt trước stylesheet để những widget không được stylesheet
    // phủ tới (hộp thoại chọn màu, tooltip...) vẫn đúng tông tối.
    QPalette pal;
    pal.setColor(QPalette::Window, kPanel);
    pal.setColor(QPalette::WindowText, kText);
    pal.setColor(QPalette::Base, kPanelAlt);
    pal.setColor(QPalette::AlternateBase, kPanel);
    pal.setColor(QPalette::Text, kText);
    pal.setColor(QPalette::Button, kPanelAlt);
    pal.setColor(QPalette::ButtonText, kText);
    pal.setColor(QPalette::ToolTipBase, kPanelAlt);
    pal.setColor(QPalette::ToolTipText, kText);
    pal.setColor(QPalette::Highlight, kAccent);
    pal.setColor(QPalette::HighlightedText, kBackground);
    pal.setColor(QPalette::Disabled, QPalette::WindowText, kTextDim);
    pal.setColor(QPalette::Disabled, QPalette::Text, kTextDim);
    pal.setColor(QPalette::Disabled, QPalette::ButtonText, kTextDim);
    app.setPalette(pal);

    const QString qss = QStringLiteral(R"(
/* ---------- Nền chung ---------- */
QWidget {
    background-color: #1D2023;
    color: #C6CCD2;
}
QMainWindow, QMainWindow > QWidget {
    background-color: #16181A;
}
QToolTip {
    background-color: #23272A;
    color: #C6CCD2;
    border: 1px solid #31373C;
    padding: 3px 6px;
}

/* ---------- Khung panel ---------- */
QFrame#PanelFrame {
    background-color: #1D2023;
    border: 1px solid #31373C;
}

/* Tiêu đề nhỏ ở đầu mỗi nhóm cài đặt */
QLabel#SectionTitle {
    color: #4FB0D8;
    font-weight: bold;
    padding: 2px 0 2px 0;
}
QLabel#Dim {
    color: #828B93;
}
QLabel#Placeholder {
    color: #6B747C;
    font-style: italic;
}

/* Đường kẻ ngăn giữa các nhóm */
QFrame#Separator {
    background-color: #2B3136;
    max-height: 1px;
    border: none;
}

/* ---------- Tab ---------- */
QTabWidget::pane {
    background-color: #1D2023;
    border: 1px solid #31373C;
    top: -1px;
}
QTabBar {
    background-color: #16181A;
    qproperty-drawBase: 0;
}
QTabBar::tab {
    background-color: #1A1D20;
    color: #8D959C;
    border: 1px solid #31373C;
    border-bottom: none;
    padding: 6px 14px;
    margin-right: 2px;
}
QTabBar::tab:hover {
    background-color: #23272A;
    color: #C6CCD2;
}
QTabBar::tab:selected {
    background-color: #1D2023;
    color: #4FB0D8;
    border-top: 2px solid #4FB0D8;
    padding-top: 5px;
}

/* ---------- Nút bấm ---------- */
QPushButton {
    background-color: #262B2F;
    color: #C6CCD2;
    border: 1px solid #3A4147;
    padding: 5px 14px;
    min-height: 18px;
}
QPushButton:hover {
    background-color: #2F353A;
    border-color: #4FB0D8;
}
QPushButton:pressed {
    background-color: #1F2427;
}
QPushButton:disabled {
    background-color: #202427;
    color: #5C646B;
    border-color: #2B3136;
}

/* Nút thoát chương trình — tông đỏ để không bấm nhầm */
QPushButton#ExitButton {
    background-color: #2E2022;
    color: #D98A8A;
    border: 1px solid #5A3336;
    font-weight: bold;
}
QPushButton#ExitButton:hover {
    background-color: #4A2A2D;
    color: #F0AFAF;
    border-color: #9A4A50;
}
QPushButton#ExitButton:pressed {
    background-color: #241A1B;
}

/* ---------- Ô nhập ---------- */
QLineEdit, QSpinBox, QDoubleSpinBox, QComboBox {
    background-color: #23272A;
    color: #C6CCD2;
    border: 1px solid #3A4147;
    padding: 3px 6px;
    selection-background-color: #4FB0D8;
    selection-color: #16181A;
}
QLineEdit:focus, QSpinBox:focus, QDoubleSpinBox:focus, QComboBox:focus {
    border-color: #4FB0D8;
}
QSpinBox::up-button, QSpinBox::down-button,
QDoubleSpinBox::up-button, QDoubleSpinBox::down-button {
    background-color: #2C3236;
    border: none;
    width: 15px;
}
QSpinBox::up-button:hover, QSpinBox::down-button:hover,
QDoubleSpinBox::up-button:hover, QDoubleSpinBox::down-button:hover {
    background-color: #3A4147;
}

/* ---------- CheckBox / RadioButton ---------- */
QCheckBox, QRadioButton {
    color: #C6CCD2;
    spacing: 7px;
    padding: 2px 0;
}
QCheckBox::indicator, QRadioButton::indicator {
    width: 13px;
    height: 13px;
    background-color: #23272A;
    border: 1px solid #4A5158;
}
QRadioButton::indicator {
    border-radius: 7px;
}
QCheckBox::indicator:hover, QRadioButton::indicator:hover {
    border-color: #4FB0D8;
}
QCheckBox::indicator:checked {
    background-color: #4FB0D8;
    border-color: #4FB0D8;
    image: none;
}
QRadioButton::indicator:checked {
    background-color: #4FB0D8;
    border: 3px solid #23272A;
}

/* ---------- Thanh trượt ---------- */
QSlider::groove:horizontal {
    background-color: #2B3136;
    height: 3px;
}
QSlider::sub-page:horizontal {
    background-color: #4FB0D8;
    height: 3px;
}
QSlider::handle:horizontal {
    background-color: #C6CCD2;
    border: 1px solid #16181A;
    width: 10px;
    height: 14px;
    margin: -6px 0;
}
QSlider::handle:horizontal:hover {
    background-color: #FFFFFF;
}

/* ---------- Bảng ---------- */
QTableWidget, QTableView {
    background-color: #1A1D20;
    alternate-background-color: #1F2326;
    color: #C6CCD2;
    gridline-color: #2B3136;
    border: 1px solid #31373C;
    selection-background-color: #2A4C5C;
    selection-color: #FFFFFF;
}
QHeaderView::section {
    background-color: #262B2F;
    color: #9DA5AC;
    border: none;
    border-right: 1px solid #31373C;
    border-bottom: 1px solid #31373C;
    padding: 4px 6px;
    font-weight: bold;
}
QTableCornerButton::section {
    background-color: #262B2F;
    border: none;
}

/* ---------- Thanh cuộn ---------- */
QScrollBar:vertical {
    background-color: #1A1D20;
    width: 10px;
    margin: 0;
}
QScrollBar::handle:vertical {
    background-color: #3A4147;
    min-height: 24px;
}
QScrollBar::handle:vertical:hover {
    background-color: #4A5158;
}
QScrollBar:horizontal {
    background-color: #1A1D20;
    height: 10px;
    margin: 0;
}
QScrollBar::handle:horizontal {
    background-color: #3A4147;
    min-width: 24px;
}
QScrollBar::handle:horizontal:hover {
    background-color: #4A5158;
}
QScrollBar::add-line, QScrollBar::sub-line {
    height: 0;
    width: 0;
}
QScrollBar::add-page, QScrollBar::sub-page {
    background: none;
}
QScrollArea {
    border: none;
}
QScrollArea > QWidget > QWidget {
    background-color: #1D2023;
}

/* ---------- Thanh trạng thái ---------- */
QStatusBar {
    background-color: #16181A;
    color: #9DA5AC;
    border-top: 1px solid #31373C;
}
QStatusBar::item {
    border: none;
}
)");

    app.setStyleSheet(qss);
}
