#
# X-01 — Phần mềm màn hình trắc thủ ra đa
#
# Yêu cầu tối thiểu: Qt 6.4 (phiên bản có sẵn trên Ubuntu 24.04) + trình biên dịch C++17.
# Trên Windows dùng Qt 6.8 + MSVC 2022 (xem .github/workflows/build.yml).
#
cmake_minimum_required(VERSION 3.21)

project(X-01
    VERSION 0.1.0
    DESCRIPTION "Phan mem man hinh trac thu ra da"
    LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

# Chỉ cần Qt Widgets — không phụ thuộc thư viện ngoài nào khác,
# nhờ đó build sạch trên cả Windows lẫn Ubuntu.
find_package(Qt6 6.4 REQUIRED COMPONENTS Widgets)

qt_standard_project_setup()

set(X01_SOURCES
    src/main.cpp

    src/MainWindow.h
    src/MainWindow.cpp
    src/AppSettings.h
    src/AppSettings.cpp
    src/Theme.h
    src/Theme.cpp

    src/map/ShapefileReader.h
    src/map/ShapefileReader.cpp
    src/map/DbfReader.h
    src/map/DbfReader.cpp
    src/map/PrjProjection.h
    src/map/PrjProjection.cpp
    src/map/MapData.h
    src/map/MapData.cpp
    src/map/MapTheme.h
    src/map/MapTheme.cpp
    src/map/MapWidget.h
    src/map/MapWidget.cpp

    src/panels/GeneralSettingsTab.h
    src/panels/GeneralSettingsTab.cpp
    src/panels/ColorsTab.h
    src/panels/ColorsTab.cpp
    src/panels/ListTab.h
    src/panels/ListTab.cpp
    src/panels/ConnectionTab.h
    src/panels/ConnectionTab.cpp
    src/panels/AmplitudeWidget.h
    src/panels/AmplitudeWidget.cpp
    src/panels/ColorButton.h
    src/panels/ColorButton.cpp
)

qt_add_executable(X-01 ${X01_SOURCES})

target_include_directories(X-01 PRIVATE src)
target_link_libraries(X-01 PRIVATE Qt6::Widgets)

# ---- Biểu tượng chương trình ----
#
# Tệp gốc là resources/app.ico (7 kích thước từ 16x16 đến 256x256). Các ảnh bên
# trong .ico vốn đã ở dạng PNG nên được tách sẵn ra từng tệp riêng và nhúng vào
# file chạy dưới dạng tài nguyên Qt. Dùng PNG thay vì đọc thẳng .ico để không
# phụ thuộc plugin đọc ảnh ICO — Qt hỗ trợ PNG sẵn trong lõi, chắc chắn có ở cả
# hai nền tảng. QIcon tự chọn kích thước khớp nhất với nơi cần hiển thị.
qt_add_resources(X-01 "appicon"
    PREFIX "/icons"
    BASE "resources"
    FILES
        resources/app-16.png
        resources/app-24.png
        resources/app-32.png
        resources/app-48.png
        resources/app-64.png
        resources/app-128.png
        resources/app-256.png
)

# Riêng Windows cần thêm .ico gắn vào chính file .exe qua tệp tài nguyên .rc,
# thì Explorer và thanh tác vụ mới hiện icon (tài nguyên Qt chỉ có tác dụng
# bên trong chương trình khi đã chạy).
if(WIN32)
    target_sources(X-01 PRIVATE resources/app.rc)
    # Trình biên dịch tài nguyên phải tìm được app.ico mà app.rc tham chiếu tới.
    # CMake truyền các thư mục include của target sang cả trình biên dịch .rc,
    # nên khai báo ở đây là đủ cho mọi bộ công cụ trên Windows.
    target_include_directories(X-01 PRIVATE resources)
endif()

# WIN32_EXECUTABLE: không mở cửa sổ console đen kèm theo trên Windows.
# RUNTIME_OUTPUT_DIRECTORY: đặt file chạy về cùng một chỗ (build/bin) cho cả
# generator một cấu hình (Ninja/Make) lẫn nhiều cấu hình (Visual Studio),
# để script CI biết chắc đường dẫn.
set_target_properties(X-01 PROPERTIES
    WIN32_EXECUTABLE TRUE
    MACOSX_BUNDLE FALSE
    RUNTIME_OUTPUT_DIRECTORY         "${CMAKE_BINARY_DIR}/bin"
    RUNTIME_OUTPUT_DIRECTORY_DEBUG   "${CMAKE_BINARY_DIR}/bin"
    RUNTIME_OUTPUT_DIRECTORY_RELEASE "${CMAKE_BINARY_DIR}/bin"
    RUNTIME_OUTPUT_DIRECTORY_RELWITHDEBINFO "${CMAKE_BINARY_DIR}/bin"
    RUNTIME_OUTPUT_DIRECTORY_MINSIZEREL     "${CMAKE_BINARY_DIR}/bin")

# Bật cảnh báo đầy đủ để lỗi lộ ra sớm ngay trên github Actions.
if(MSVC)
    target_compile_options(X-01 PRIVATE /W4 /utf-8 /permissive-)
else()
    target_compile_options(X-01 PRIVATE -Wall -Wextra -Wpedantic)
endif()
