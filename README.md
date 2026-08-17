# ESP8266 with IR 

Một Project nhỏ mà tôi vọc vạch được trong cái hè=))))

**Tính năng chính:**
-  Giao diện web đơn giản (Web vẫn còn một số lỗi nhỏ mà tôi không biết sửa=((( )
-  Điều khiển qua WiFi Access Point (AP)
-  Thu nhận và phát các mã IR (NEC, RC5, RC6,...)
-  Tương tác thời gian thực, không cần reload
-  Hỗ trợ điều khiển qua Serial Monitor

---

## Hardwares (Phần cứng cần có)

Dưới đây là các thiết bị cần thiết để xây dựng dự án. Bấm vào từng liên kết để xem chi tiết sản phẩm:

| # | Thiết bị | Mô tả | Link đặt hàng |
|---|---------|-------|---------------|
| 1 | **ESP8266 NodeMCU** | Mạch để điều khiển Wifi | [Đặt hàng](https://s.shopee.vn/5ArlDZRV3h) |
| 2 | **Cảm biến IR Receiver** (VS1838B) | Thu tín hiệu hồng ngoại | [Đặt hàng](https://s.shopee.vn/1VySqw6s1m) |
| 3 | **LED IR Transmitter** (940nm) | Phát tín hiệu hồng ngoại | [Đặt hàng](https://s.shopee.vn/9fKAZzaucW) |
| 4 | **Dây USB Micro** | Cấp điện & nạp code cho ESP8266 | [Đặt hàng](https://s.shopee.vn/30nGdmkzG2) |
| *Để đảm bảo an toàn cho LED IR - Đây là linh kiện cần có (Bạn sợ à=))) )* | | | |            
| 5 | **Breadboard** | Bảng giác cắm | [Đặt hàng](https://s.shopee.vn/qim3vrvf0) |
| 6 | **Dây cắm (Cái-Cái)** | Nối linh kiện | [Đặt hàng](https://s.shopee.vn/9KhKBGnfeO) |
| 7 | **Trở** | Dùng để hạ áp | [Đặt hàng](https://s.shopee.vn/9zx0ytNYmc) |

**Sơ đồ kết nối:**
```
IR Receiver (VS1838B)
  ├─ VCC → 3.3V (ESP8266)
  ├─ GND → GND (ESP8266)
  └─ OUT → D5 (GPIO14 - IR_RECEIVE_PIN)

IR LED Transmitter (940nm)
  ├─ (+) → D2 (GPIO4 - IR_SEND_PIN) qua transistor/FET
  └─ (-) → GND (ESP8266)
```

---

## Driver 

Trước tiên, bạn cần cài đặt driver CH341 để kết nối ESP8266 với máy tính:

### Bước 1: Tải Driver
Tải driver CH341 cho hệ điều hành của bạn:

**Windows:**
- Tải tại: [CH341 Driver Download](https://www.wch-ic.com/downloads/CH341SER_EXE.html)
- Chạy file `.exe` và làm theo hướng dẫn cài đặt

**macOS/Linux:**
```bash
# Thường đã được hỗ trợ sẵn, hoặc cài qua package manager:
# macOS (Homebrew):
brew install ch341
```

### Bước 2: Kiểm tra Driver
Sau khi cài đặt, kết nối ESP8266 vào máy tính:
- **Windows**: Kiểm tra Device Manager → Ports (COM & LPT)
- **macOS/Linux**: Chạy lệnh `ls /dev/tty.*` hoặc `ls /dev/ttyUSB*`

---

## Setup 

### Bước 1: Cài đặt Arduino IDE

1. Tải Arduino IDE từ: [arduino.cc/software](https://www.arduino.cc/software/)
2. Cài đặt và mở Arduino IDE

### Bước 2: Cài đặt Board ESP8266

1. Mở **Arduino IDE → File → Preferences**
2. Tìm mục **Additional Boards Manager URLs**
3. Dán URL sau:
   ```
   http://arduino.esp8266.com/stable/package_esp8266com_index.json
   ```
4. Bấm **OK**
5. Mở **Tools → Board → Boards Manager...**
6. Tìm: `esp8266` (bởi ESP8266 Community)
7. Bấm **Install** phiên bản mới nhất

### Bước 3: Cài đặt thư viện IRremote

1. Mở **Sketch → Include Library → Manage Libraries...**
2. Tìm: `IRremote`
3. Cài đặt phiên bản `≥ 4.0` (do Arno Steffens)

### Bước 4: Thiết lập kết nối

1. **Kết nối ESP8266** với máy tính qua USB
2. Mở **Tools → Board** → Chọn **ESP8266 → Generic ESP8266 Module**
3. **Tools → Port** → Chọn COM port mới nhất (ví dụ: `COM3`)
4. Tùy chọn khác:
   - Upload Speed: `921600`
   - Flash Size: `4MB (FS: 2MB OTA: ~1019KB)`

### Bước 5: Cấu hình WiFi

Mở file `ir_receiver.ino` và tìm dòng 7-8:

```cpp
const char* ap_ssid = "wifi_name";  // ← Đổi thành tên WiFi của bạn
const char* ap_pass = "passwords";  // ← Đổi thành mật khẩu của bạn
```

**Ví dụ:**
```cpp
const char* ap_ssid = "Xin_chao_123";
const char* ap_pass = "12345678";
```

### Bước 6: Nạp code

1. Mở file `ir_receiver.ino`
2. Bấm nút **Upload** (mũi tên sang phải) hoặc `Ctrl + U`
3. Chờ thông báo `Leaving... Hard resetting via RTS pin...`

### Bước 7: Truy cập giao diện web

1. Mở **Tools → Serial Monitor** (Ctrl + Shift + M)
2. Thiết lập **Baud rate: 115200**
3. Bạn sẽ thấy:
   ```
   =============================================
                      IR SCANNER
   =============================================
    Wi-Fi: Smart_IR_Hub
    IP   : http://192.168.4.1
   =============================================
   ```
4. Kết nối WiFi từ thiết bị (điện thoại/laptop) đến WiFi: `Smart_IR_Hub`
5. Mở trình duyệt và truy cập: **http://192.168.4.1**

---

## Hướng dẫn sử dụng 

### Thu hồng ngoại từ remote

1. Nhập **tên nút** (ví dụ: "Bật TV")
2. Chọn **vị trí Slot** (1-50) để lưu mã
3. Bấm **"BẮT ĐẦU THU HỒNG NGOẠI"**
4. Hướng remote vào cảm biến và bấm nút trên remote
5. Chờ thông báo "Đã lưu thành công!"

### Phát lại mã IR

**Cách 1: Từ giao diện web**
- Bấm nút tương ứng trong "Bảng Điều Khiển"

**Cách 2: Phát nhanh thủ công**
- Chọn Slot trong mục "Phát Nhanh Thủ Công" → Bấm **PHÁT**

**Cách 3: Từ Serial Monitor**
- Gõ lệnh: `send 1` (phát mã ở Slot 1)
- Gõ `list` để xem danh sách mã đã lưu

### Quản lý mã hồng ngoại

- **Xem chi tiết**: Bấm vào nút trong "Bảng Điều Khiển"
- **Đổi tên**: Chỉnh sửa tên → Bấm **ĐỔI TÊN**
- **Xóa**: Bấm **XÓA** (không thể hoàn tác)

---

## Các lệnh Serial Monitor

Kết nối qua Serial Monitor (115200 baud) để sử dụng các lệnh:

| Lệnh | Mô tả |
|------|-------|
| `help` | Hiển thị danh sách lệnh |
| `list` | Danh sách tất cả mã đã lưu |
| `send 1` | Phát mã ở Slot 1 |
| `erase 1` | Xóa mã ở Slot 1 |

---

## Ghi chú & Khắc phục sự cố

### ⚠️ ESP8266 không được nhận dạng
- Kiểm tra driver CH341 đã cài đặt chưa
- Thử USB port khác hoặc cáp USB khác
- Restart Arduino IDE

### ⚠️ Không thể upload code
- Kiểm tra baud rate upload: `921600`
- Kiểm tra board đã chọn: `Generic ESP8266 Module`
- Kiểm tra COM port đúng

### ⚠️ WiFi không xuất hiện
- Kiểm tra mật khẩu WiFi ≥ 8 ký tự
- Kiểm tra phần cứng ESP8266 hoạt động (LED LED bật lên)

### ⚠️ Cảm biến IR không hoạt động
- Kiểm tra kết nối GPIO D5 (pin 14)
- Thử với remote khác
- Kiểm tra thư viện IRremote phiên bản ≥ 4.0

---

## Tham khảo & Liên kết

-  **GitHub mà tôi tham khảo**: [mdhiggins/ESP8266-HTTP-IR-Blaster](https://github.com/mdhiggins/ESP8266-HTTP-IR-Blaster)
-  **IRremote Library**: [z3t0/Arduino-IRremote](https://github.com/z3t0/Arduino-IRremote)
-  **ESP8266 Docs**: [esp8266.com](https://www.esp8266.com/)

---

**⚠️⚠️**

Nếu gặp vấn đề, hãy kiểm tra Serial Monitor hoặc tạo issue trên repository.
