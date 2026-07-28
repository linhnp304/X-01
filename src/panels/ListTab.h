#pragma once

#include <QWidget>

class QPushButton;
class QTableWidget;

/// Tab "Danh sách" — bảng quỹ đạo và bảng điểm dấu.
///
/// Bước 1 mới dựng khung giao diện: hai bảng trống đúng cột và nút xoá danh sách
/// điểm dấu. Phần nạp/cập nhật dữ liệu sẽ làm ở bước sau.
class ListTab : public QWidget
{
    Q_OBJECT

public:
    explicit ListTab(QWidget *parent = nullptr);

private:
    void buildUi();

    QTableWidget *m_trackTable = nullptr;
    QTableWidget *m_markTable = nullptr;
    QPushButton *m_clearMarksBtn = nullptr;
};
