#include "panels/AmplitudeWidget.h"

#include "Theme.h"

#include <QFont>
#include <QPainter>

AmplitudeWidget::AmplitudeWidget(QWidget *parent)
    : QWidget(parent)
{
    setAutoFillBackground(false);
    setMinimumHeight(80);
}

void AmplitudeWidget::paintEvent(QPaintEvent *event)
{
    Q_UNUSED(event)

    QPainter p(this);
    const QRect r = rect();

    // Nền tối hơn panel điều khiển để cửa sổ biên độ tách bạch rõ
    p.fillRect(r, QColor(0x14, 0x16, 0x18));

    // Lưới mờ làm nền: 10 cột, 4 hàng
    QPen grid(QColor(0x26, 0x2C, 0x31));
    grid.setWidth(1);
    p.setPen(grid);
    for (int i = 1; i < 10; ++i) {
        const int x = r.left() + r.width() * i / 10;
        p.drawLine(x, r.top(), x, r.bottom());
    }
    for (int i = 1; i < 4; ++i) {
        const int y = r.top() + r.height() * i / 4;
        p.drawLine(r.left(), y, r.right(), y);
    }

    // Viền ngoài
    p.setPen(Theme::kBorder);
    p.drawRect(r.adjusted(0, 0, -1, -1));

    // Tiêu đề góc trên bên trái
    QFont f;
    f.setFamilies(Theme::fontFamilies());
    f.setPointSize(9);
    f.setBold(true);
    p.setFont(f);
    p.setPen(Theme::kAccent);
    p.drawText(r.adjusted(10, 6, -10, 0), Qt::AlignLeft | Qt::AlignTop, tr("CỬA SỔ BIÊN ĐỘ"));

    // Ghi chú giữa khung
    f.setBold(false);
    f.setItalic(true);
    p.setFont(f);
    p.setPen(QColor(0x6B, 0x74, 0x7C));
    p.drawText(r, Qt::AlignCenter, tr("(Sẽ làm ở bước sau)"));
}
