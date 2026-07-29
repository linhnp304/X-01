#include "map/MapTheme.h"

#include <QtGlobal>

namespace {

/// Nội suy tuyến tính giữa hai màu RGB dạng 0xRRGGBB theo hệ số t trong [0, 1].
QColor lerp(quint32 dark, quint32 light, double t)
{
    auto mix = [t](int a, int b) {
        return static_cast<int>(a + (b - a) * t + 0.5);
    };
    const int r = mix((dark >> 16) & 0xFF, (light >> 16) & 0xFF);
    const int g = mix((dark >> 8) & 0xFF, (light >> 8) & 0xFF);
    const int b = mix(dark & 0xFF, light & 0xFF);
    return QColor(r, g, b);
}

} // namespace

MapTheme MapTheme::fromBrightness(int brightness)
{
    const int b = qBound(1, brightness, 10);
    const double t = (b - 1) / 9.0; // 0 = tối nhất, 1 = sáng nhất

    MapTheme th;
    th.sea      = lerp(0x17181A, 0x8A8D8E, t);
    th.land     = lerp(0x1E1F21, 0x767879, t);
    th.province = lerp(0x2B2D30, 0x54575A, t);
    th.nation   = lerp(0xA0702D, 0xD9A44F, t);
    th.coast    = lerp(0x2A7C96, 0x52B9D4, t);
    // Sông ngòi lấy tông nước giống bờ biển nhưng trầm hơn, để đường bờ biển
    // vẫn là nét nước nổi bật nhất trên bản đồ.
    th.river    = lerp(0x235A69, 0x4894A8, t);
    th.label    = lerp(0x3D4145, 0x2E3134, t);
    th.placeDot = lerp(0x45494E, 0x232527, t);

    // Đường bay dân dụng và sân bay được nâng sáng/tăng tương phản so với bản tham khảo
    // theo yêu cầu, để dễ nhìn hơn ở các mức sáng giữa (xem Screens/maps_03, maps_04).
    // Ở đầu tối thì làm sáng lên; ở đầu sáng (nền đất xám nhạt) thì làm đậm lại
    // để nét vẫn nổi trên nền — cả hai đều nhằm tăng độ rõ.
    th.airRoute      = lerp(0x55697A, 0x86A0B4, t);
    th.airRouteLabel = lerp(0x62788A, 0x54626E, t);
    th.airport       = lerp(0x7A858E, 0x2B3034, t);

    return th;
}
