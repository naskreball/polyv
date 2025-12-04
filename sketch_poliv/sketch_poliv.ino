#include <WiFi.h>

//        НАСТРОЙКИ СЕТИ
const char* ssid = "PolivSystem";
const char* password = "12345678";
const int RELAY_PIN = 27;
const int MOISTURE_PIN = 34;
const int LED_PIN = 2;

//      ГЛОБАЛЬНЫЕ ПЕРЕМЕННЫЕ
WiFiServer server(8888);
WiFiClient client;
bool clientConnected = false;

bool pumpState = false;
bool autoMode = false;
unsigned long pumpStartTime = 0;
const unsigned long MAX_PUMP_TIME = 3000;

//        НАСТРОЙКИ АВТОПОЛИВА 
unsigned long lastAutoCheck = 0;
const unsigned long AUTO_CHECK_INTERVAL = 10000;
int DRY_THRESHOLD = 2000;
int WET_THRESHOLD = 1000;

//      НАСТРОЙКА 
void setup() {
    Serial.begin(115200);
    delay(1000);
    
    Serial.println("\n\n=== ESP32 Poliv System ===");
    
    // Настройка пинов
    pinMode(RELAY_PIN, OUTPUT);
    digitalWrite(RELAY_PIN, HIGH);
    pinMode(LED_PIN, OUTPUT);
    
    // Wi-Fi точка доступа
    WiFi.mode(WIFI_AP);
    WiFi.softAP(ssid, password);
    delay(1000);
    
    server.begin();
    
    Serial.println("=== СИСТЕМА ГОТОВА ===");
    Serial.print("SSID: ");
    Serial.println(ssid);
    Serial.print("IP: ");
    Serial.println(WiFi.softAPIP());
    Serial.println("Ожидаю подключения...");
    
    digitalWrite(LED_PIN, HIGH);
}

//  ЧТЕНИЕ ВЛАЖНОСТИ 
int readMoisture() {
    return analogRead(MOISTURE_PIN);
}

//  ВКЛЮЧЕНИЕ ПОМПЫ 
void startPump() {
    if (pumpState) return;
    
    Serial.println(">>> ВКЛЮЧАЮ ПОМПУ <<<");
    digitalWrite(RELAY_PIN, LOW);
    pumpState = true;
    pumpStartTime = millis();
}

//  ВЫКЛЮЧЕНИЕ ПОМПЫ 
void stopPump() {
    if (!pumpState) return;
    
    Serial.println(">>> ВЫКЛЮЧАЮ ПОМПУ <<<");
    digitalWrite(RELAY_PIN, HIGH);
    pumpState = false;
}

//  ПРОВЕРКА АВТОПОЛИВА
void checkAutoWatering() {
    if (!autoMode) return;
    
    int moisture = readMoisture();
    Serial.print("Автополив: влажность=");
    Serial.print(moisture);
    Serial.print(" пороги: сухо>");
    Serial.print(DRY_THRESHOLD);
    Serial.print(" влажно<");
    Serial.println(WET_THRESHOLD);
    
    if (moisture > DRY_THRESHOLD && !pumpState) {
        Serial.println("!!! СУХО - ВКЛЮЧАЮ ПОЛИВ !!!");
        startPump();
    } else if (moisture < WET_THRESHOLD && pumpState) {
        Serial.println("!!! ВЛАЖНО - ВЫКЛЮЧАЮ ПОЛИВ !!!");
        stopPump();
    }
}

//       ОБРАБОТКА КОМАНД 
String processCommand(String cmd) {
    cmd.trim();
    Serial.print("Обработка команды: ");
    Serial.println(cmd);
    
    if (cmd == "PING") {
        return "PONG";
    }
    else if (cmd == "STATUS") {
        int moisture = readMoisture();
        String status = "Влажность:";
        status += String(moisture);
        status += " | Помпа:";
        status += pumpState ? "ВКЛ" : "ВЫКЛ";
        status += " | Авто:";
        status += autoMode ? "ВКЛ" : "ВЫКЛ";
        return status;
    }
    else if (cmd == "SOIL") {
        int moisture = readMoisture();
        return String(moisture);  // ТОЛЬКО число!
    }
    else if (cmd == "PUMP_ON") {
        if (pumpState) return "Помпа уже включена";
        startPump();
        return "Помпа включена (макс 3 сек)";
    }
    else if (cmd == "PUMP_OFF") {
        if (!pumpState) return "Помпа уже выключена";
        stopPump();
        return "Помпа выключена";
    }
    else if (cmd == "SHORT_PUMP") {
        if (pumpState) return "Помпа занята";
        
        digitalWrite(RELAY_PIN, LOW);
        delay(1000);
        digitalWrite(RELAY_PIN, HIGH);
        
        return "Короткий полив 1 сек";
    }
    else if (cmd == "AUTO_ON") {
        autoMode = true;
        Serial.println("Автополив ВКЛЮЧЕН");
        return "Автополив включен";
    }
    else if (cmd == "AUTO_OFF") {
        autoMode = false;
        Serial.println("Автополив ВЫКЛЮЧЕН");
        return "Автополив выключен";
    }
    else if (cmd == "TEST") {
        return "ESP32 работает OK";
    }
    else if (cmd == "GET_INFO") {
        String info = "SSID:";

info += ssid;
        info += "|IP:";
        info += WiFi.softAPIP().toString();
        info += "|Влажность:";
        info += String(readMoisture());
        return info;
    }
    
    return "Неизвестная команда: " + cmd;
}

//  ОБРАБОТКА КЛИЕНТА
void handleClient() {
    // Проверяем нового клиента
    if (!client || !client.connected()) {
        client = server.available();
        if (client) {
            Serial.println(">>> Клиент подключился!");
            clientConnected = true;
            digitalWrite(LED_PIN, LOW);
        }
        return;
    }
    
    // Читаем данные от клиента
    if (client.available()) {
        String cmd = client.readStringUntil('\n');
        
        if (cmd.length() > 0) {
            String response = processCommand(cmd);
            client.println(response);
            Serial.print("Отправлен ответ: ");
            Serial.println(response);
        }
    }
    
    // Если клиент отключился
    if (!client.connected()) {
        Serial.println("<<< Клиент отключился");
        client.stop();
        clientConnected = false;
        digitalWrite(LED_PIN, HIGH);
    }
}

//  ГЛАВНЫЙ ЦИКЛ 
void loop() {
    // 1. Автоотключение помпы
    if (pumpState && millis() - pumpStartTime > MAX_PUMP_TIME) {
        Serial.println("Автоотключение помпы (3 секунды)");
        stopPump();
    }
    
    // 2. Проверка автополива
    if (millis() - lastAutoCheck > AUTO_CHECK_INTERVAL) {
        lastAutoCheck = millis();
        checkAutoWatering();
    }
    
    // 3. Обработка клиента
    handleClient();
    
    // 4. Мигание LED если нет клиента
    if (!clientConnected) {
        static unsigned long lastBlink = 0;
        if (millis() - lastBlink > 1000) {
            lastBlink = millis();
            digitalWrite(LED_PIN, !digitalRead(LED_PIN));
        }
    }
    
    delay(50);
}
