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

    // Trên Linux chạy nền Wayland, trình quản lý cửa sổ ghép icon theo tên tệp
    // .desktop chứ không lấy từ setWindowIcon. Khai báo sẵn tên để nếu sau này
    // có cài tệp X-01.desktop thì icon hiện đúng cả ở thanh tác vụ.
    QGuiApplication::setDesktopFileName(QStringLiteral("X-01"));

    // Giao diện tối kiểu kỹ thuật, thống nhất trên cả Windows và Ubuntu
    Theme::apply(app);

    MainWindow w;
    // Yêu cầu: chạy chương trình ở chế độ toàn màn hình (bấm F11 để thoát/vào lại)
    w.showFullScreen();

    return app.exec();
}
