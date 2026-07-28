#pragma once

#include <QWidget>

class QPushButton;
class QTableWidget;

/// Tab "Kết nối" — danh sách các cổng TCP/UDP gửi nhận dữ liệu.
///
/// Bước 1 mới dựng khung giao diện: bảng cổng trống và nút Kết nối/Dừng kết nối.
/// Cấu hình cổng và giao thức gói tin sẽ mô tả ở bước 2.
class ConnectionTab : public QWidget
{
    Q_OBJECT

public:
    explicit ConnectionTab(QWidget *parent = nullptr);

    /// Đang ở trạng thái kết nối hay chưa.
    bool isConnected() const { return m_connected; }

signals:
    /// Người dùng bấm nút Kết nối/Dừng kết nối.
    void connectionToggled(bool connected);

private:
    void buildUi();
    void toggleConnection();

    QTableWidget *m_portTable = nullptr;
    QPushButton *m_connectBtn = nullptr;
    bool m_connected = false;
};
