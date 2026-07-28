#pragma once

#include <QWidget>

class AppSettings;
class ColorButton;

/// Tab "Màu sắc" — quy ước màu sắc của các đối tượng trên màn hình chính.
/// Mọi màu người dùng đổi đều được ghi ngay ra tệp cấu hình .json.
class ColorsTab : public QWidget
{
    Q_OBJECT

public:
    explicit ColorsTab(AppSettings *settings, QWidget *parent = nullptr);

    /// Nạp màu từ AppSettings lên các ô chọn màu.
    void loadFromSettings();

signals:
    void settingsChanged();

private:
    void buildUi();

    AppSettings *m_settings = nullptr;

    ColorButton *m_markPoint = nullptr;
    ColorButton *m_track = nullptr;
    ColorButton *m_trackInfo = nullptr;
    ColorButton *m_sweep = nullptr;
};
