# X-01 — Phần mềm màn hình trắc thủ ra đa

Phần mềm hiển thị tình huống trên nền bản đồ số, viết bằng **Qt 6 / C++17**, build bằng **CMake**,
chạy được trên **Windows 10 trở lên** và **Ubuntu 24.04 trở lên** từ cùng một mã nguồn.

Giao diện theo phong cách kỹ thuật, gam màu tối.

---

## 1. Bố cục màn hình

Chương trình chạy ở chế độ toàn màn hình (thiết kế cho độ phân giải 1920×1080), chia làm 3 panel:

```
+---------------------------------------+---------------------------+
|                                       |  Panel 2.1  (75% chiều dọc)|
|                                       |  Danh sách | Kết nối |     |
|        Panel 1 — bản đồ số            |  Màu sắc  | Cài đặt chung |
|        (70% chiều ngang)              |                           |
|                                       +---------------------------+
|                          [− ▭▭▭▭ +]   |  Panel 2.2  (25% chiều dọc)|
|                                       |  Cửa sổ biên độ            |
+---------------------------------------+---------------------------+
|  Panel 3 — thanh trạng thái                                       |
+-------------------------------------------------------------------+
```

**Panel 1 — màn hình hiển thị chính**

* Nền bản đồ số vẽ từ thư mục `MapChuan` (shapefile, không dùng thư viện bản đồ ngoài).
* Phóng to/thu nhỏ bằng thanh trượt góc dưới bên phải hoặc con lăn chuột
  (giữ nguyên điểm địa lý nằm dưới con trỏ).
* Bấm giữ chuột trái và kéo để đổi khung nhìn — con trỏ đổi thành hình bàn tay nắm giữ.
* Vòng tròn cự ly, đường chia độ và đường quét ra đa vẽ đè lên nền bản đồ.

**Panel 2 — cột điều khiển**

| Tab | Nội dung |
|---|---|
| Danh sách | Bảng quỹ đạo, bảng điểm dấu, nút xoá danh sách điểm dấu *(dữ liệu làm ở bước sau)* |
| Kết nối | Bảng cổng TCP/UDP, nút Kết nối/Dừng kết nối *(làm ở bước sau)* |
| Màu sắc | Màu điểm dấu, quỹ đạo, thông tin quỹ đạo, đường quét ra đa |
| Cài đặt chung | Tab mở mặc định — xem mục 2 |

**Panel 3 — thanh trạng thái**

* Bên trái: trạng thái kết nối đến các máy tính khác *(làm ở bước sau)*.
* Ở giữa: thời gian hệ thống + toạ độ con trỏ chuột khi di trên bản đồ.
* Bên phải: toạ độ tâm đài.

Mọi toạ độ đều hiển thị dạng lat-lng, làm tròn đến 0,000001 độ.

---

## 2. Tab "Cài đặt chung"

* **Độ sáng bản đồ**: thanh trượt 1 (tối nhất) → 10 (sáng nhất).
* **Ẩn/Hiện các lớp bản đồ**: Đường bay dân dụng, Sân bay, Tên địa danh, Địa phận tỉnh/thành phố.
* **Toạ độ tâm đài**: ô nhập lat/lng + nút **Áp dụng** (dịch tâm đài và đưa khung nhìn về vị trí mới).
* **Cự ly tối đa (km)**: bán kính vòng tròn cự ly ngoài cùng — mặc định **50 km**.
* **Vòng tròn cự ly**: 5km / 1km / 0.5km / 0.1km / Tắt.

  | Chế độ | Vòng đậm | Vòng mảnh |
  |---|---|---|
  | 5km | mỗi 5 km | — |
  | 1km | mỗi 5 km | mỗi 1 km |
  | 0.5km | mỗi 1 km | mỗi 0,5 km |
  | 0.1km | mỗi 0,5 km | mỗi 0,1 km |

* **Đường chia độ**: 30 độ / 10 độ / 5 độ / Tắt (đường đậm là bước lớn, đường mảnh là bước nhỏ).
* **Hiện thông tin quỹ đạo**.
* **Tự động xoá quỹ đạo khi mất cập nhật** + số giây (mặc định 60).

Toàn bộ cài đặt và màu sắc người dùng thay đổi được ghi ngay ra tệp `X-01.json`
nằm cùng thư mục file chạy, và trở thành mặc định cho các lần chạy sau.

---

## 3. Bố trí thư mục khi chạy

```
<thư mục gốc>/
├── X-01(.exe)      ← file chạy
├── X-01.json       ← tệp cấu hình (chương trình tự tạo sau lần chạy đầu)
└── MapChuan/       ← dữ liệu bản đồ số (tự copy vào)
```

Nếu chưa có thư mục `MapChuan`, phần mềm vẫn chạy bình thường nhưng nền bản đồ để trống
và hiện dòng nhắc ở giữa panel 1.

---

## 4. Chạy phần mềm

### Windows

Tải artifact `X-01-windows` từ tab **Actions** của repo, giải nén, copy thư mục `MapChuan`
vào cùng chỗ với `X-01.exe` rồi chạy. Gói đã kèm sẵn thư viện Qt, không cần cài thêm gì.

### Ubuntu 24.04

Tải artifact `X-01-ubuntu-24.04`, sau đó:

```bash
sudo apt install libqt6widgets6
```

```bash
chmod +x X-01 && ./X-01
```

---

## 5. Phím tắt

| Phím | Chức năng |
|---|---|
| `F11` | Bật/tắt chế độ toàn màn hình |

---

## 6. Tự build trên máy

Chưa cần thiết ở giai đoạn này (github Actions đã build sẵn cho cả 2 nền tảng).
Khi cần build tại chỗ thì cài các công cụ sau:

**Windows**: Visual Studio 2022 (workload *Desktop development with C++*), CMake ≥ 3.21,
Qt 6.5+ bản `msvc2022_64`.

**Ubuntu 24.04**:

```bash
sudo apt install build-essential cmake ninja-build qt6-base-dev qt6-base-dev-tools libgl1-mesa-dev
```

Lệnh build (giống nhau ở cả hai nền tảng):

```bash
cmake -B build -S . -DCMAKE_BUILD_TYPE=Release && cmake --build build --config Release --parallel
```

File chạy nằm ở `build/bin/`.

---

## 7. Cấu trúc mã nguồn

```
src/
├── main.cpp                    Điểm vào, bật chế độ toàn màn hình
├── MainWindow.*                Ghép 3 panel, thanh trạng thái, nối tín hiệu
├── AppSettings.*               Đọc/ghi cấu hình ra X-01.json
├── Theme.*                     Bảng màu + stylesheet tối cho toàn giao diện
├── map/
│   ├── ShapefileReader.*       Đọc tệp .shp (Point/PolyLine/Polygon + biến thể Z/M)
│   ├── DbfReader.*             Đọc thuộc tính từ tệp .dbf
│   ├── PrjProjection.*         Nghịch đảo phép chiếu Lambert Conformal Conic và Transverse Mercator
│   ├── MapData.*               Nạp MapChuan, dựng hình học 3 mức chi tiết theo zoom
│   ├── MapTheme.*              Bảng màu nền bản đồ theo độ sáng 1..10
│   └── MapWidget.*             Panel 1: vẽ bản đồ, vòng cự ly, đường chia độ, đường quét
└── panels/
    ├── GeneralSettingsTab.*    Tab Cài đặt chung
    ├── ColorsTab.*             Tab Màu sắc
    ├── ListTab.*               Tab Danh sách (khung)
    ├── ConnectionTab.*         Tab Kết nối (khung)
    ├── ColorButton.*           Ô chọn màu dùng chung
    └── AmplitudeWidget.*       Panel 2.2: cửa sổ biên độ (khung)
```

### Ghi chú kỹ thuật

* **Phép chiếu hiển thị**: equirectangular — `x = kinh độ`, `y = -vĩ độ`.
  Vì vậy một vòng tròn cự ly thật trên mặt đất hiện lên dưới dạng hình elip hơi bè ngang,
  tỉ lệ `1/cos(vĩ độ tâm đài)`. Đây là cách vẽ đúng về mặt hình học khi phủ lên nền bản đồ.
* **Mức chi tiết (LOD)**: dữ liệu gốc rất nặng (riêng `VNM_adm1.shp` hơn 6 MB), nên mỗi lớp
  hình học được lược bớt điểm sẵn thành 3 mức; khi vẽ chỉ chọn mức đủ mịn so với tỉ lệ hiện tại.
  Các lớp đường còn được loại nhanh phần nằm ngoài khung nhìn bằng hình chữ nhật bao.
