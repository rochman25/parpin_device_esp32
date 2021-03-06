#include <Arduino.h>

#include <WiFiMulti.h>
#include <WebSocketsClient.h>
#include <ArduinoJson.h>

WiFiMulti WifiMulti;
WebSocketsClient WebSocket;

volatile int flow_frequency;
unsigned int l_hour;
unsigned int flowsensor = 22;
unsigned long currentTime;
unsigned long cloopTime;

void flow()
{
  flow_frequency++;
}

//websocket event
void webSocketEvent(WStype_t type, uint8_t *payload, size_t length)
{
  String message = "";
  const int capacity = JSON_ARRAY_SIZE(2) + 4 * JSON_OBJECT_SIZE(2);
  StaticJsonDocument<capacity> doc;
  doc["t"] = 1;
  JsonObject data = doc.createNestedObject("d");
  data["topic"] = "alat:5ebe4cd46246ed22f9afc08f";
  serializeJson(doc, message);

  switch (type)
  {
  case WStype_DISCONNECTED:
    digitalWrite(BUILTIN_LED, LOW);
    Serial.printf("[WSc] Disconnected! \n");
    delay(500);
    digitalWrite(BUILTIN_LED, HIGH);
    delay(500);
    break;
  case WStype_CONNECTED:
  {
    digitalWrite(BUILTIN_LED, HIGH);
    Serial.printf("[WSc] Connected to url: %s\n", payload);
    WebSocket.sendTXT(message);
  }
  break;
  case WStype_TEXT:
    Serial.printf("[WSc] get text: %s\n", payload);
    break;
  // case WStype_BIN:
  //   Serial.printf("[WSc] get binary length: %u\n", length);
  //   hexdump(payload, length);
  //   break;
  case WStype_PING:
    // pong will be send automatically
    Serial.printf("[WSc] get ping\n");
    break;
  case WStype_PONG:
    // answer to a ping we send
    Serial.printf("[WSc] get pong\n");
    // USE_SERIAL.printf("[WSc] get text: %s\n", payload);
    break;
  default:
    Serial.printf("[WSc] get type: %u\n", type);
    break;
  }
}

void setup()
{
  // put your setup code here, to run once:

  Serial.begin(115200);
  Serial.setDebugOutput(true);
  Serial.println();
  Serial.println();

  for (uint8_t t = 1; t < 4; t++)
  {
    Serial.printf("[SETUP] Booting %d...\n", t);
    Serial.flush();
    delay(1000);
  }

  WifiMulti.addAP("Private", "12345678");
  while (WifiMulti.run() != WL_CONNECTED)
  {
    pinMode(BUILTIN_LED, OUTPUT);
    digitalWrite(BUILTIN_LED, LOW);
    Serial.printf("[SETUP] Network failed \n");
    delay(1000);
    digitalWrite(BUILTIN_LED, HIGH);
    delay(1000);
  }

  WebSocket.begin("192.168.43.73", 3333, "/adonis-ws");

  WebSocket.onEvent(webSocketEvent);

  //waktu untuk connect kembali dengan server
  WebSocket.setReconnectInterval(5000);

  //setting disconnect if pong not received 5 times / untuk optimal koneksi websocket
  WebSocket.enableHeartbeat(15000, 3000, 5);

  pinMode(flowsensor, INPUT);
  digitalWrite(flowsensor, HIGH);
  attachInterrupt(flowsensor, flow, RISING);
  sei();
  currentTime = millis();
  cloopTime = currentTime;
}

void loop()
{
  // put your main code here, to run repeatedly:
  if (WifiMulti.run() != WL_CONNECTED)
  {
    // digitalWrite(LED, LOW);
    pinMode(BUILTIN_LED, OUTPUT);
    digitalWrite(BUILTIN_LED, LOW);
    Serial.printf("[SETUP] Network failed \n");
    delay(1000);
    digitalWrite(BUILTIN_LED, HIGH);
    delay(1000);
  }
  else
  {
    currentTime = millis();
    if (currentTime >= (cloopTime + 1000)){
      String message = "";
      const int capacity = JSON_ARRAY_SIZE(2) + 4 * JSON_OBJECT_SIZE(2);
      StaticJsonDocument<capacity> doc;
      doc["t"] = 7;
      JsonObject data = doc.createNestedObject("d");
      data["topic"] = "alat:5ebe4cd46246ed22f9afc08f";
      data["event"] = "message";
      JsonObject msg = data.createNestedObject("data");
      msg["alat_id"] = "5ebe4cd46246ed22f9afc08f";
      msg["nama_alat"] = "Alat 2";
      msg["status"] = "online";
      cloopTime = currentTime;
      l_hour = (flow_frequency * 60 / 7.5);
      flow_frequency = 0;
      Serial.print(l_hour, DEC);
      Serial.println(" L/hour");
      msg["arus"] = l_hour;
      serializeJson(doc, message);
      WebSocket.sendTXT(message);
    }
    WebSocket.loop();
    delay(2000);
  }
}