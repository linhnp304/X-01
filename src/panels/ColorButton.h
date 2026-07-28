#pragma once

#include <QColor>
#include <QPushButton>

/// Ô chọn màu: hiện một mảng màu kèm mã hex, bấm vào thì mở hộp thoại chọn màu.
class ColorButton : public QPushButton
{
    Q_OBJECT

public:
    explicit ColorButton(QWidget *parent = nullptr);

    QColor color() const { return m_color; }
    /// Đặt màu mà không phát tín hiệu (dùng khi nạp cấu hình lúc khởi động).
    void setColor(const QColor &c);

    /// Tiêu đề hộp thoại chọn màu.
    void setDialogTitle(const QString &title) { m_dialogTitle = title; }

signals:
    /// Người dùng vừa chọn một màu mới.
    void colorPicked(const QColor &c);

protected:
    void paintEvent(QPaintEvent *event) override;
    QSize sizeHint() const override;

private:
    void openPicker();

    QColor m_color{ Qt::white };
    QString m_dialogTitle;
};
