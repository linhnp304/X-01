#include "panels/ListTab.h"

#include <QHBoxLayout>
#include <QHeaderView>
#include <QLabel>
#include <QPushButton>
#include <QTableWidget>
#include <QVBoxLayout>

namespace {

QLabel *sectionTitle(const QString &text)
{
    auto *l = new QLabel(text);
    l->setObjectName(QStringLiteral("SectionTitle"));
    return l;
}

/// Dựng một bảng chỉ đọc với các cột cho trước.
QTableWidget *makeTable(const QStringList &headers, QWidget *parent)
{
    auto *t = new QTableWidget(0, headers.size(), parent);
    t->setHorizontalHeaderLabels(headers);
    t->verticalHeader()->setVisible(false);
    t->horizontalHeader()->setSectionResizeMode(QHeaderView::Stretch);
    t->setSelectionBehavior(QAbstractItemView::SelectRows);
    t->setSelectionMode(QAbstractItemView::SingleSelection);
    t->setEditTriggers(QAbstractItemView::NoEditTriggers);
    t->setAlternatingRowColors(true);
    t->setShowGrid(false);
    return t;
}

} // namespace

ListTab::ListTab(QWidget *parent)
    : QWidget(parent)
{
    buildUi();
}

void ListTab::buildUi()
{
    auto *root = new QVBoxLayout(this);
    root->setContentsMargins(12, 12, 12, 12);
    root->setSpacing(6);

    // ---- Danh sách quỹ đạo ----
    root->addWidget(sectionTitle(tr("Danh sách quỹ đạo")));
    m_trackTable = makeTable({ tr("Số tốp"), tr("Cự ly (km)"), tr("Phương vị (độ)"),
                               tr("Độ cao (m)"), tr("Tốc độ (km/h)") },
                             this);
    root->addWidget(m_trackTable, 3);

    // ---- Danh sách điểm dấu ----
    root->addSpacing(4);
    root->addWidget(sectionTitle(tr("Danh sách điểm dấu")));
    m_markTable = makeTable({ tr("STT"), tr("Vĩ độ"), tr("Kinh độ"), tr("Thời gian") }, this);
    root->addWidget(m_markTable, 2);

    m_clearMarksBtn = new QPushButton(tr("Xoá danh sách điểm dấu"), this);
    auto *btnRow = new QHBoxLayout;
    btnRow->setContentsMargins(0, 0, 0, 0);
    btnRow->addStretch(1);
    btnRow->addWidget(m_clearMarksBtn);
    root->addLayout(btnRow);

    auto *note = new QLabel(tr("(Phần dữ liệu của tab này sẽ làm ở bước sau)"), this);
    note->setObjectName(QStringLiteral("Placeholder"));
    root->addWidget(note);

    // Chưa có dữ liệu để xoá nên tạm khoá nút
    m_clearMarksBtn->setEnabled(false);
}
