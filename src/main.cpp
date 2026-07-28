#include "MainWindow.h"
#include "Theme.h"

#include <QApplication>
#include <QIcon>

int main(int argc, char *argv[])
{
    QApplication app(argc, argv);

    QCoreApplication::setApplicationName(QStringLiteral("X-01"));
    QCoreApplication::setApplicationVersion(QStringLiteral("0.1.0"));
    QCoreApplication::setOrganizationName(QStringLiteral("X-01"));

    // Biểu tượng chương trình: nạp đủ các kích thước đã nhúng, QIcon sẽ tự chọn
    // ảnh khớp nhất cho từng chỗ (thanh tiêu đề, thanh tác vụ, trình chuyển cửa sổ).
    QIcon appIcon;
    for (int size : { 16, 24, 32, 48, 64, 128, 256 })
        appIcon.addFile(QStringLiteral(":/icons/app-%1.png").arg(size));
    QApplication::setWindowIcon(appIcon);

    // Chú ý: KHÔNG gọi QGuiApplication::setDesktopFileName ở đây.
    // Trên Linux, Qt đã tự đăng ký định danh chương trình với dịch vụ
    // xdg-desktop-portal ngay lúc khởi động; gọi thêm setDesktopFileName sẽ kích
    // hoạt lần đăng ký thứ hai trên cùng một kết nối D-Bus và sinh cảnh báo
    // "Could not register app ID: Connection already associated with an
    // application ID" trên cửa sổ dòng lệnh. Hàm này chỉ có tác dụng khi hệ thống
    // đã cài sẵn tệp X-01.desktop — hiện chưa cài nên bỏ đi là đúng.

    // Giao diện tối kiểu kỹ thuật, thống nhất trên cả Windows và Ubuntu
    Theme::apply(app);

    MainWindow w;
    // Yêu cầu: chạy chương trình ở chế độ toàn màn hình (bấm F11 để thoát/vào lại)
    w.showFullScreen();

    return app.exec();
}
