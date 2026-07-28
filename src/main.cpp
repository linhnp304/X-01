#include "MainWindow.h"
#include "Theme.h"

#include <QApplication>

int main(int argc, char *argv[])
{
    QApplication app(argc, argv);

    QCoreApplication::setApplicationName(QStringLiteral("X-01"));
    QCoreApplication::setApplicationVersion(QStringLiteral("0.1.0"));
    QCoreApplication::setOrganizationName(QStringLiteral("X-01"));

    // Giao diện tối kiểu kỹ thuật, thống nhất trên cả Windows và Ubuntu
    Theme::apply(app);

    MainWindow w;
    // Yêu cầu: chạy chương trình ở chế độ toàn màn hình (bấm F11 để thoát/vào lại)
    w.showFullScreen();

    return app.exec();
}
