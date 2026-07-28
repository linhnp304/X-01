#include "panels/ColorsTab.h"

#include "AppSettings.h"
#include "panels/ColorButton.h"

#include <QFormLayout>
#include <QFrame>
#include <QLabel>
#include <QVBoxLayout>

namespace {

QLabel *sectionTitle(const QString &text)
{
    auto *l = new QLabel(text);
    l->setObjectName(QStringLiteral("SectionTitle"));
    return l;
}

QFrame *separator()
{
    auto *f = new QFrame;
    f->setObjectName(QStringLiteral("Separator"));
    f->setFrameShape(QFrame::HLine);
    f->setFixedHeight(1);
    return f;
}

} // namespace

ColorsTab::ColorsTab(AppSettings *settings, QWidget *parent)
    : QWidget(parent)
    , m_settings(settings)
{
    buildUi();
    loadFromSettings();
}

void ColorsTab::buildUi()
{
    auto *root = new QVBoxLayout(this);
    root->setContentsMargins(12, 12, 12, 12);
    root->setSpacing(8);

    root->addWidget(sectionTitle(tr("Quy ước màu sắc của các đối tượng")));

    m_markPoint = new ColorButton(this);
    m_markPoint->setDialogTitle(tr("Màu điểm dấu"));

    m_track = new ColorButton(this);
    m_track->setDialogTitle(tr("Màu quỹ đạo"));

    m_trackInfo = new ColorButton(this);
    m_trackInfo->setDialogTitle(tr("Màu thông tin quỹ đạo"));

    auto *form = new QFormLayout;
    form->setContentsMargins(0, 0, 0, 0);
    form->setLabelAlignment(Qt::AlignLeft | Qt::AlignVCenter);
    form->setSpacing(8);
    form->addRow(tr("Điểm dấu:"), m_markPoint);
    form->addRow(tr("Quỹ đạo:"), m_track);
    form->addRow(tr("Thông tin quỹ đạo:"), m_trackInfo);
    root->addLayout(form);

    root->addSpacing(6);
    root->addWidget(separator());
    root->addWidget(sectionTitle(tr("Màn hình chính")));

    m_sweep = new ColorButton(this);
    m_sweep->setDialogTitle(tr("Màu đường quét ra đa"));

    auto *form2 = new QFormLayout;
    form2->setContentsMargins(0, 0, 0, 0);
    form2->setLabelAlignment(Qt::AlignLeft | Qt::AlignVCenter);
    form2->setSpacing(8);
    form2->addRow(tr("Đường quét ra đa:"), m_sweep);
    root->addLayout(form2);

    auto *note = new QLabel(tr("Vòng tròn cự ly và đường chia độ dùng chung màu với "
                               "đường quét ra đa (vẽ mờ hơn)."), this);
    note->setObjectName(QStringLiteral("Dim"));
    note->setWordWrap(true);
    root->addWidget(note);

    connect(m_markPoint, &ColorButton::colorPicked, this, [this](const QColor &c) {
        m_settings->colorMarkPoint = c;
        emit settingsChanged();
    });
    connect(m_track, &ColorButton::colorPicked, this, [this](const QColor &c) {
        m_settings->colorTrack = c;
        emit settingsChanged();
    });
    connect(m_trackInfo, &ColorButton::colorPicked, this, [this](const QColor &c) {
        m_settings->colorTrackInfo = c;
        emit settingsChanged();
    });
    connect(m_sweep, &ColorButton::colorPicked, this, [this](const QColor &c) {
        m_settings->colorSweep = c;
        emit settingsChanged();
    });

    root->addStretch(1);
}

void ColorsTab::loadFromSettings()
{
    // setColor() không phát tín hiệu nên không cần chặn tín hiệu ở đây
    m_markPoint->setColor(m_settings->colorMarkPoint);
    m_track->setColor(m_settings->colorTrack);
    m_trackInfo->setColor(m_settings->colorTrackInfo);
    m_sweep->setColor(m_settings->colorSweep);
}
