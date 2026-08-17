#include <Arduino.h>
#include <IRremote.hpp>
#include <EEPROM.h>
#include <ESP8266WiFi.h>
#include <ESP8266WebServer.h>

const char* ap_ssid = "wifi_name"; 
const char* ap_pass = "passwords";          
ESP8266WebServer server(80);

const int IR_RECEIVE_PIN = D5; 
const int IR_SEND_PIN = D2;    
const int EEPROM_SIZE = 2048;   
const int MAX_SAVED_CODES = 50;
const int EEPROM_START_ADDR = 0;

struct IRCode {
  uint8_t protocol;
  uint16_t address;
  uint16_t command;
  uint8_t bits;
  char name[24]; 
};

unsigned long lastReceiveTime = 0;
uint32_t frameCount = 0;
uint16_t lastCommand = 0;
uint8_t lastProtocol = 0;

bool isWaitingForIR = false;
int targetRecordSlot = 1;
String targetRecordName = "";

const char index_html[] PROGMEM = R"rawliteral(
<!DOCTYPE html>
<html lang="vi">
<head>
  <meta charset="UTF-8">
  <meta name="viewport" content="width=device-width, initial-scale=1">
  <title>Smart IR Hub Pro</title>
  <style>
    :root { --bg: #f1f5f9; --card: #ffffff; --primary: #3b82f6; --danger: #ef4444; --success: #10b981; --info: #0ea5e9; --text: #1e293b; --text-light: #64748b; }
    body { font-family: 'Segoe UI', system-ui, sans-serif; background: var(--bg); color: var(--text); margin: 0; padding: 15px; display: flex; flex-direction: column; align-items: center; }
    .container { width: 100%; max-width: 480px; }
    .card { background: var(--card); border-radius: 16px; padding: 20px; box-shadow: 0 4px 6px -1px rgba(0,0,0,0.1); margin-bottom: 20px; }
    h2, h3 { margin-top: 0; border-bottom: 2px solid var(--bg); padding-bottom: 10px; font-size: 18px; }
    .input-group { margin-bottom: 15px; }
    label { display: block; margin-bottom: 6px; font-size: 13px; font-weight: 600; color: var(--text-light); }
    input { width: 100%; padding: 12px; border: 1px solid #cbd5e1; border-radius: 8px; box-sizing: border-box; font-size: 15px; outline: none; transition: 0.2s; }
    input:focus { border-color: var(--primary); box-shadow: 0 0 0 3px rgba(59,130,246,0.1); }
    .btn { width: 100%; padding: 12px; color: white; border: none; border-radius: 8px; font-size: 14px; font-weight: bold; cursor: pointer; transition: transform 0.1s; }
    .btn:active { transform: scale(0.96); }
    .btn-record { background: var(--text); }
    .btn-send { background: var(--primary); display: flex; flex-direction: column; align-items: center; box-shadow: 0 4px 6px rgba(59,130,246,0.2); }
    .btn-send small { font-size: 11px; opacity: 0.8; font-weight: normal; margin-top: 4px; }
    .btn-green { background: var(--success); }
    .btn-blue { background: var(--info); }
    .btn-red { background: var(--danger); }
    .btn-gray { background: #e2e8f0; color: var(--text); }
    .grid { display: grid; grid-template-columns: 1fr 1fr; gap: 12px; }
    .flex-row { display: flex; gap: 10px; }
    .status-box { display: none; margin-top: 15px; padding: 12px; background: #fef08a; color: #854d0e; border-radius: 8px; text-align: center; font-weight: 600; font-size: 14px; animation: pulse 1.5s infinite; }
    .empty-state { text-align: center; color: var(--text-light); font-size: 14px; padding: 20px 0; }
    @keyframes pulse { 0% { opacity: 1; } 50% { opacity: 0.5; } 100% { opacity: 1; } }
    .overlay { display: none; position: fixed; top: 0; left: 0; right: 0; bottom: 0; background: rgba(15,23,42,0.6); backdrop-filter: blur(4px); z-index: 100; align-items: center; justify-content: center; }
    .modal { background: white; width: 90%; max-width: 340px; border-radius: 16px; padding: 24px; box-shadow: 0 20px 25px -5px rgba(0,0,0,0.1); animation: pop 0.3s cubic-bezier(0.175, 0.885, 0.32, 1.275); }
    @keyframes pop { from { transform: scale(0.8); opacity: 0; } to { transform: scale(1); opacity: 1; } }
    .modal-title-input { width: 100%; font-size: 18px; font-weight: bold; color: var(--primary); text-align: center; border: 1px dashed #cbd5e1; border-radius: 8px; padding: 10px; margin-bottom: 15px; box-sizing: border-box; }
    .modal-title-input:focus { border: 1px solid var(--primary); }
    .info-row { display: flex; justify-content: space-between; padding: 8px 0; border-bottom: 1px dashed #e2e8f0; font-size: 14px; }
    .info-row span:last-child { font-family: monospace; font-weight: 600; color: var(--primary); }
    .modal-actions { display: grid; grid-template-columns: 1fr 1fr; gap: 10px; margin-top: 20px; }
  </style>
</head>
<body>
  <div class="container">
    <div class="card">
      <h2> Bảng Điều Khiển</h2>
      <div id="remoteGrid" class="grid"></div>
      <div id="emptyText" class="empty-state">Chưa có mã hồng ngoại nào được lưu.</div>
    </div>

    <div class="card">
      <h3> Phát Nhanh Thủ Công</h3>
      <div class="flex-row">
        <input type="number" id="manualSlot" placeholder="Nhập Slot (1-50)" min="1" max="50">
        <button class="btn btn-green" onclick="sendManual()" style="width: 100px;">PHÁT</button>
      </div>
    </div>

    <div class="card">
      <h3> Học Lệnh & Lưu Slot</h3>
      <div class="input-group">
        <label>Tên nút (VD: Bật Điều Hòa)</label>
        <input type="text" id="btnName" placeholder="Nhập tên nút...">
      </div>
      <div class="input-group">
        <label>Vị trí Slot lưu trữ (1 - 50)</label>
        <input type="number" id="btnSlot" value="1" min="1" max="50">
      </div>
      <button class="btn btn-record" onclick="startRecord()">BẮT ĐẦU THU HỒNG NGOẠI</button>
      <div id="statusBox" class="status-box">⏳ Đang chờ tín hiệu... Hãy bấm Remote!</div>
    </div>
  </div>

  <div id="infoModal" class="overlay">
    <div class="modal">
      <input type="text" id="m_name_input" class="modal-title-input" placeholder="Tên Nút">
      <div class="info-row"><span>Slot lưu trữ:</span> <span id="m_slot">1</span></div>
      <div class="info-row"><span>Giao thức:</span> <span id="m_protocol">NEC</span></div>
      <div class="info-row"><span>Địa chỉ:</span> <span id="m_address">0x00</span></div>
      <div class="info-row"><span>Mã lệnh:</span> <span id="m_command">0x00</span></div>
      <div class="info-row"><span>Độ dài:</span> <span id="m_bits">32</span></div>
      <div class="modal-actions">
        <button class="btn btn-green" onclick="actionSend()">PHÁT</button>
        <button class="btn btn-blue" onclick="actionRename()">ĐỔI TÊN</button>
        <button class="btn btn-red" onclick="actionDelete()">XÓA</button>
        <button class="btn btn-gray" onclick="closeModal()">ĐÓNG</button>
      </div>
    </div>
  </div>

  <script>
    let currentSelectedSlot = 0;

    function loadButtons() {
      fetch('/list_json').then(r => r.json()).then(data => {
        const grid = document.getElementById('remoteGrid');
        const emptyText = document.getElementById('emptyText');
        grid.innerHTML = '';
        if(data.length === 0) {
          emptyText.style.display = 'block';
        } else {
          emptyText.style.display = 'none';
          data.forEach(item => {
            const btn = document.createElement('button');
            btn.className = 'btn btn-send';
            btn.innerHTML = `<span>${item.name}</span><small>Slot ${item.slot}</small>`;
            btn.onclick = () => openModal(item);
            grid.appendChild(btn);
          });
        }
      });
    }

    function openModal(item) {
      currentSelectedSlot = item.slot;
      document.getElementById('m_name_input').value = item.name;
      document.getElementById('m_slot').innerText = item.slot;
      document.getElementById('m_protocol').innerText = item.protocol;
      document.getElementById('m_address').innerText = item.address;
      document.getElementById('m_command').innerText = item.command;
      document.getElementById('m_bits').innerText = item.bits;
      document.getElementById('infoModal').style.display = 'flex';
    }

    function closeModal() {
      document.getElementById('infoModal').style.display = 'none';
    }

    function actionSend() {
      fetch('/send?id=' + currentSelectedSlot);
      closeModal();
    }

    function actionRename() {
      const newName = document.getElementById('m_name_input').value;
      if (newName.trim() === '') {
        alert('Tên không được để trống!');
        return;
      }
      fetch(`/rename?id=${currentSelectedSlot}&name=${encodeURIComponent(newName)}`).then(() => {
        closeModal();
        loadButtons();
      });
    }

    function actionDelete() {
      if(confirm('Bạn có chắc chắn muốn xóa nút này vĩnh viễn?')) {
        fetch('/delete?id=' + currentSelectedSlot).then(() => {
          closeModal();
          loadButtons();
        });
      }
    }

    function sendManual() {
      const slot = document.getElementById('manualSlot').value;
      if (slot >= 1 && slot <= 50) fetch('/send?id=' + slot);
      else alert("Vui lòng nhập Slot từ 1 đến 50!");
    }

    let pollInterval;
    function startRecord() {
      const name = document.getElementById('btnName').value || 'Không tên';
      const slot = document.getElementById('btnSlot').value || 1;
      
      document.getElementById('statusBox').style.display = 'block';
      
      fetch(`/start_record?slot=${slot}&name=${encodeURIComponent(name)}`).then(() => {
        pollInterval = setInterval(() => {
          fetch('/status').then(r => r.text()).then(state => {
            if(state === 'idle') {
              clearInterval(pollInterval);
              document.getElementById('statusBox').style.display = 'none';
              alert('Đã thu và lưu lệnh thành công!');
              loadButtons(); 
            }
          });
        }, 1000);
      });
    }

    window.onload = loadButtons;
  </script>
</body>
</html>
)rawliteral";

void setup() {
  Serial.begin(115200);
  Serial.setTimeout(50);
  
  EEPROM.begin(EEPROM_SIZE); 
  
  IrReceiver.begin(IR_RECEIVE_PIN, ENABLE_LED_FEEDBACK);
  IrSender.begin(IR_SEND_PIN, ENABLE_LED_FEEDBACK);

  WiFi.mode(WIFI_AP);
  WiFi.softAP(ap_ssid, ap_pass);

  server.on("/", []() {
    server.send_P(200, "text/html", index_html);
  });

  server.on("/send", []() {
    if (server.hasArg("id")) {
      sendCode(server.arg("id").toInt());
      server.send(200, "text/plain", "OK");
    } else {
      server.send(400, "text/plain", "Bad Request");
    }
  });

  server.on("/delete", []() {
    if (server.hasArg("id")) {
      eraseCode(server.arg("id").toInt());
      server.send(200, "text/plain", "OK");
    } else {
      server.send(400, "text/plain", "Bad Request");
    }
  });

  server.on("/rename", []() {
    if (server.hasArg("id") && server.hasArg("name")) {
      renameCode(server.arg("id").toInt(), server.arg("name"));
      server.send(200, "text/plain", "OK");
    } else {
      server.send(400, "text/plain", "Bad Request");
    }
  });

  server.on("/start_record", []() {
    if (server.hasArg("slot") && server.hasArg("name")) {
      targetRecordSlot = server.arg("slot").toInt();
      targetRecordName = server.arg("name");
      isWaitingForIR = true; 
      Serial.println(F("Dang cho thu tin hieu hong ngoai..."));
      server.send(200, "text/plain", "OK");
    }
  });

  server.on("/status", []() {
    server.send(200, "text/plain", isWaitingForIR ? "recording" : "idle");
  });

  server.on("/list_json", []() {
    String json = "[";
    bool first = true;
    for (int i = 0; i < MAX_SAVED_CODES; i++) {
      IRCode code;
      EEPROM.get(EEPROM_START_ADDR + (i * sizeof(IRCode)), code);
      if (code.protocol > 0 && code.protocol < 255) {
        if (!first) json += ",";
        
        String protoName = String((const __FlashStringHelper*)getProtocolString((decode_type_t)code.protocol));
        
        json += "{\"slot\":" + String(i + 1) + 
                ",\"name\":\"" + String(code.name) + 
                "\",\"protocol\":\"" + protoName + 
                "\",\"address\":\"0x" + String(code.address, HEX) + 
                "\",\"command\":\"0x" + String(code.command, HEX) + 
                "\",\"bits\":" + String(code.bits) + "}";
        first = false;
      }
    }
    json += "]";
    server.send(200, "application/json", json);
  });

  server.begin();
  printHeader();
}

void loop() {
  server.handleClient(); 

  if (IrReceiver.decode()) {
    processIRData();
    
    if (isWaitingForIR && IrReceiver.decodedIRData.protocol != UNKNOWN && IrReceiver.decodedIRData.protocol != 0) {
      saveCode(targetRecordSlot, targetRecordName);
      isWaitingForIR = false; 
      Serial.println(F("Da thu va luu thanh cong!"));
    }
    IrReceiver.resume(); 
  }

  if (Serial.available() > 0) {
    String cmd = Serial.readStringUntil('\n');
    cmd.trim();
    if (cmd.length() > 0) handleSerialCommand(cmd);
  }
}

void processIRData() {
  unsigned long currentTime = millis();
  unsigned long timeBetweenFrames = currentTime - lastReceiveTime;
  
  bool isRepeat = (IrReceiver.decodedIRData.flags & IRDATA_FLAGS_IS_REPEAT);
  if (isRepeat && timeBetweenFrames < 250) {
    lastReceiveTime = currentTime;
    return; 
  }
  frameCount++;
  
  Serial.print(F("Quet duoc Protocol: "));
  Serial.print((const __FlashStringHelper*)getProtocolString(IrReceiver.decodedIRData.protocol));
  Serial.print(F(" | Command: 0x")); Serial.println(IrReceiver.decodedIRData.command, HEX);

  lastReceiveTime = currentTime;
  lastCommand = IrReceiver.decodedIRData.command;
  lastProtocol = IrReceiver.decodedIRData.protocol;
}

void handleSerialCommand(String cmd) {
  cmd.toLowerCase();
  if (cmd == F("help")) printHeader();
  else if (cmd == F("list")) listSavedCodes();
  else if (cmd.startsWith(F("save "))) saveCode(cmd.substring(5).toInt(), "Serial_Saved"); 
  else if (cmd.startsWith(F("send "))) sendCode(cmd.substring(5).toInt());
  else if (cmd.startsWith(F("erase "))) eraseCode(cmd.substring(6).toInt());
}

void saveCode(int id, String name) {
  if (id < 1 || id > MAX_SAVED_CODES) return;

  IRCode codeToSave = {lastProtocol, IrReceiver.decodedIRData.address, IrReceiver.decodedIRData.command, IrReceiver.decodedIRData.numberOfBits, ""};
  
  strncpy(codeToSave.name, name.c_str(), sizeof(codeToSave.name) - 1);
  codeToSave.name[sizeof(codeToSave.name) - 1] = '\0'; 

  int addr = EEPROM_START_ADDR + ((id - 1) * sizeof(IRCode));
  EEPROM.put(addr, codeToSave);
  EEPROM.commit(); 
  
  Serial.print(F("Da luu ma vao Slot ")); Serial.println(id);
}

void renameCode(int id, String newName) {
  if (id < 1 || id > MAX_SAVED_CODES) return;
  IRCode code;
  int addr = EEPROM_START_ADDR + ((id - 1) * sizeof(IRCode));
  EEPROM.get(addr, code);
  if (code.protocol == 0 || code.protocol == 255) return;

  strncpy(code.name, newName.c_str(), sizeof(code.name) - 1);
  code.name[sizeof(code.name) - 1] = '\0';
  
  EEPROM.put(addr, code);
  EEPROM.commit();
  Serial.print(F("Da doi ten Slot ")); Serial.println(id);
}

void listSavedCodes() {
  Serial.println(F("\n--- DANH SACH MA DA LUU ---"));
  for (int i = 0; i < MAX_SAVED_CODES; i++) {
    IRCode code;
    EEPROM.get(EEPROM_START_ADDR + (i * sizeof(IRCode)), code);
    if (code.protocol > 0 && code.protocol < 255) {
      Serial.print(F("Slot ")); Serial.print(i + 1); 
      Serial.print(F(" | Ten: ")); Serial.print(code.name);
      Serial.print(F(" | Cmd: 0x")); Serial.println(code.command, HEX);
    }
  }
}

void sendCode(int id) {
  if (id < 1 || id > MAX_SAVED_CODES) return;
  IRCode code;
  EEPROM.get(EEPROM_START_ADDR + ((id - 1) * sizeof(IRCode)), code);
  if (code.protocol == 0 || code.protocol == 255) return;

  Serial.print(F("Dang phat lenh: ")); Serial.println(code.name);
  
  IRData dataToSend;
  dataToSend.protocol = (decode_type_t)code.protocol;
  dataToSend.address = code.address;
  dataToSend.command = code.command;
  dataToSend.numberOfBits = code.bits;
  dataToSend.flags = 0; 

  IrSender.write(&dataToSend, 2); 
}

void eraseCode(int id) {
  if (id < 1 || id > MAX_SAVED_CODES) return;
  int addr = EEPROM_START_ADDR + ((id - 1) * sizeof(IRCode));
  for (unsigned int i = 0; i < sizeof(IRCode); i++) { EEPROM.write(addr + i, 255); }
  EEPROM.commit(); 
  Serial.print(F("Da xoa Slot ")); Serial.println(id);
}

void printHeader() {
  Serial.println(F("\n============================================="));
  Serial.println(F("                   IR SCANNER                  "));
  Serial.println(F("============================================="));
  Serial.print(F(" Wi-Fi: ")); Serial.println(ap_ssid);
  Serial.print(F(" IP   : http://")); Serial.println(WiFi.softAPIP());
  Serial.println(F("=============================================\n"));
}