#include "panels/ColorButton.h"

#include "Theme.h"

#include <QColorDialog>
#include <QPainter>

ColorButton::ColorButton(QWidget *parent)
    : QPushButton(parent)
{
    setCursor(Qt::PointingHandCursor);
    setFlat(true);
    // Cần WA_Hover thì Qt mới gửi sự kiện rê chuột để vẽ lại viền sáng
    setAttribute(Qt::WA_Hover, true);
    connect(this, &QPushButton::clicked, this, &ColorButton::openPicker);
}

void ColorButton::setColor(const QColor &c)
{
    if (!c.isValid() || c == m_color)
        return;
    m_color = c;
    update();
}

QSize ColorButton::sizeHint() const
{
    return QSize(180, 24);
}

void ColorButton::paintEvent(QPaintEvent *event)
{
    Q_UNUSED(event)

    QPainter p(this);
    p.setRenderHint(QPainter::Antialiasing, false);

    const QRect r = rect().adjusted(0, 0, -1, -1);

    // Nền + viền của ô (viền sáng lên khi rê chuột để biết bấm được)
    p.fillRect(r, Theme::kPanelAlt);
    p.setPen(underMouse() ? Theme::kAccent : Theme::kBorder);
    p.drawRect(r);

    // Mảng màu ở nửa trái
    const QRect swatch = QRect(r.left() + 4, r.top() + 4, r.width() / 2 - 8, r.height() - 8);
    p.fillRect(swatch, m_color);
    p.setPen(Theme::kBorder);
    p.drawRect(swatch);

    // Mã hex ở nửa phải
    p.setPen(Theme::kText);
    const QRect textRect(swatch.right() + 10, r.top(), r.right() - swatch.right() - 14, r.height());
    p.drawText(textRect, Qt::AlignLeft | Qt::AlignVCenter, m_color.name(QColor::HexRgb).toUpper());
}

void ColorButton::openPicker()
{
    const QColor c = QColorDialog::getColor(
        m_color, this,
        m_dialogTitle.isEmpty() ? tr("Chọn màu") : m_dialogTitle);
    if (!c.isValid() || c == m_color)
        return;
    m_color = c;
    update();
    emit colorPicked(m_color);
}
