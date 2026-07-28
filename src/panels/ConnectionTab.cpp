#include "panels/ConnectionTab.h"

#include <QHBoxLayout>
#include <QHeaderView>
#include <QLabel>
#include <QPushButton>
#include <QTableWidget>
#include <QVBoxLayout>

ConnectionTab::ConnectionTab(QWidget *parent)
    : QWidget(parent)
{
    buildUi();
}

void ConnectionTab::buildUi()
{
    auto *root = new QVBoxLayout(this);
    root->setContentsMargins(12, 12, 12, 12);
    root->setSpacing(6);

    auto *title = new QLabel(tr("Danh sách cổng gửi/nhận dữ liệu"), this);
    title->setObjectName(QStringLiteral("SectionTitle"));
    root->addWidget(title);

    m_portTable = new QTableWidget(0, 5, this);
    m_portTable->setHorizontalHeaderLabels(
        { tr("Tên"), tr("Giao thức"), tr("Địa chỉ IP"), tr("Cổng"), tr("Trạng thái") });
    m_portTable->verticalHeader()->setVisible(false);
    m_portTable->horizontalHeader()->setSectionResizeMode(QHeaderView::Stretch);
    m_portTable->setSelectionBehavior(QAbstractItemView::SelectRows);
    m_portTable->setSelectionMode(QAbstractItemView::SingleSelection);
    m_portTable->setEditTriggers(QAbstractItemView::NoEditTriggers);
    m_portTable->setAlternatingRowColors(true);
    m_portTable->setShowGrid(false);
    root->addWidget(m_portTable, 1);

    m_connectBtn = new QPushButton(tr("Kết nối"), this);
    m_connectBtn->setMinimumWidth(130);
    auto *btnRow = new QHBoxLayout;
    btnRow->setContentsMargins(0, 0, 0, 0);
    btnRow->addStretch(1);
    btnRow->addWidget(m_connectBtn);
    root->addLayout(btnRow);

    auto *note = new QLabel(tr("(Cấu hình cổng và giao thức gói tin sẽ làm ở bước sau)"), this);
    note->setObjectName(QStringLiteral("Placeholder"));
    root->addWidget(note);

    connect(m_connectBtn, &QPushButton::clicked, this, &ConnectionTab::toggleConnection);

    // Chưa có cổng nào để kết nối nên tạm khoá nút
    m_connectBtn->setEnabled(false);
}

void ConnectionTab::toggleConnection()
{
    m_connected = !m_connected;
    m_connectBtn->setText(m_connected ? tr("Dừng kết nối") : tr("Kết nối"));
    emit connectionToggled(m_connected);
}
