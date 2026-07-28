#pragma once

#include <QWidget>

/// Panel 2.2 — cửa sổ biên độ, nằm dưới cùng của cột điều khiển bên phải.
///
/// Bước 1 mới dựng khung: nền tối kèm lưới mờ và tiêu đề. Phần vẽ biên độ tín hiệu
/// sẽ làm ở bước sau.
class AmplitudeWidget : public QWidget
{
    Q_OBJECT

public:
    explicit AmplitudeWidget(QWidget *parent = nullptr);

protected:
    void paintEvent(QPaintEvent *event) override;
};
