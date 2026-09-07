#include <Arduino.h>

const int LED_PIN = 2;
const int BUZZER_PIN = 8;

const TickType_t HEARTBEAT_PERIOD = pdMS_TO_TICKS(1000);
const TickType_t STATUS_PERIOD = pdMS_TO_TICKS(1000);
const TickType_t ALERT_ON_TIME = pdMS_TO_TICKS(300);

enum AlertType {
  ALERT_NONE = 0,
  ALERT_OBJECT_DETECTED,
  ALERT_TEST
};

struct AlertMessage {
  AlertType type;
  char label[24];
  uint32_t timestampMs;
};

struct HeartbeatMessage {
  uint32_t uptimeMs;
  uint32_t freeHeap;
};

QueueHandle_t alertQueue;
QueueHandle_t heartbeatQueue;
SemaphoreHandle_t serialMutex;

volatile uint32_t objectDetectionCount = 0;
volatile uint32_t testAlertCount = 0;
volatile uint32_t heartbeatCount = 0;
volatile uint32_t lastCommandTimeMs = 0;

void safePrintln(const String &message) {
  if (xSemaphoreTake(serialMutex, pdMS_TO_TICKS(50)) == pdTRUE) {
    Serial.println(message);
    xSemaphoreGive(serialMutex);
  }
}

void pushAlert(AlertType type, const char *label) {
  AlertMessage message;
  message.type = type;
  strncpy(message.label, label, sizeof(message.label) - 1);
  message.label[sizeof(message.label) - 1] = '\0';
  message.timestampMs = millis();

  xQueueSend(alertQueue, &message, pdMS_TO_TICKS(20));
}

void HeartbeatTask(void *parameter) {
  HeartbeatMessage message;

  while (true) {
    message.uptimeMs = millis();
    message.freeHeap = ESP.getFreeHeap();

    xQueueSend(heartbeatQueue, &message, 0);
    heartbeatCount++;

    vTaskDelay(HEARTBEAT_PERIOD);
  }
}

void AlertTask(void *parameter) {
  AlertMessage message;

  while (true) {
    if (xQueueReceive(alertQueue, &message, portMAX_DELAY) == pdTRUE) {
      digitalWrite(LED_PIN, HIGH);
      digitalWrite(BUZZER_PIN, HIGH);

      if (message.type == ALERT_OBJECT_DETECTED) {
        objectDetectionCount++;
        safePrintln("ALERT: " + String(message.label) + " detected by YOLO");
      } else if (message.type == ALERT_TEST) {
        testAlertCount++;
        safePrintln("ALERT: manual test alert");
      }

      vTaskDelay(ALERT_ON_TIME);

      digitalWrite(LED_PIN, LOW);
      digitalWrite(BUZZER_PIN, LOW);
    }
  }
}

void CommunicationTask(void *parameter) {
  String command = "";

  while (true) {
    while (Serial.available() > 0) {
      char incoming = (char)Serial.read();

      if (incoming == '\n' || incoming == '\r') {
        command.trim();

        if (command.length() > 0) {
          lastCommandTimeMs = millis();

          if (command == "PERSON_DETECTED") {
            pushAlert(ALERT_OBJECT_DETECTED, "PERSON");
          } else if (command == "CHAIR_DETECTED") {
            pushAlert(ALERT_OBJECT_DETECTED, "CHAIR");
          } else if (command == "BOTTLE_DETECTED") {
            pushAlert(ALERT_OBJECT_DETECTED, "BOTTLE");
          } else if (command == "PHONE_DETECTED") {
            pushAlert(ALERT_OBJECT_DETECTED, "PHONE");
          } else if (command == "LAPTOP_DETECTED") {
            pushAlert(ALERT_OBJECT_DETECTED, "LAPTOP");
          } else if (command == "TEST_ALERT") {
            pushAlert(ALERT_TEST, "TEST");
          } else if (command == "PING") {
            safePrintln("PONG");
          } else {
            safePrintln("Unknown command: " + command);
          }
        }

        command = "";
      } else if (command.length() < 60) {
        command += incoming;
      }
    }

    vTaskDelay(pdMS_TO_TICKS(20));
  }
}

void StatusTask(void *parameter) {
  HeartbeatMessage latestHeartbeat = {0, 0};

  while (true) {
    while (xQueueReceive(heartbeatQueue, &latestHeartbeat, 0) == pdTRUE) {
    }

    String lastCommandText;
    if (lastCommandTimeMs == 0) {
      lastCommandText = "none";
    } else {
      lastCommandText = String((millis() - lastCommandTimeMs) / 1000) + " s ago";
    }

    safePrintln(
      "STATUS | Uptime: " + String(latestHeartbeat.uptimeMs / 1000) + " s" +
      " | Free heap: " + String(latestHeartbeat.freeHeap) + " bytes" +
      " | YOLO alerts: " + String(objectDetectionCount) +
      " | Test alerts: " + String(testAlertCount) +
      " | Heartbeats: " + String(heartbeatCount) +
      " | Last command: " + lastCommandText
    );

    vTaskDelay(STATUS_PERIOD);
  }
}

void setup() {
  Serial.begin(115200);
  delay(1000);

  pinMode(LED_PIN, OUTPUT);
  pinMode(BUZZER_PIN, OUTPUT);

  digitalWrite(LED_PIN, LOW);
  digitalWrite(BUZZER_PIN, LOW);

  alertQueue = xQueueCreate(10, sizeof(AlertMessage));
  heartbeatQueue = xQueueCreate(6, sizeof(HeartbeatMessage));
  serialMutex = xSemaphoreCreateMutex();

  if (alertQueue == NULL || heartbeatQueue == NULL || serialMutex == NULL) {
    Serial.println("RTOS resource creation failed");
    while (true) {
      delay(1000);
    }
  }

  xTaskCreate(HeartbeatTask, "Heartbeat Task", 3072, NULL, 1, NULL);
  xTaskCreate(AlertTask, "Alert Task", 4096, NULL, 4, NULL);
  xTaskCreate(CommunicationTask, "Communication Task", 4096, NULL, 3, NULL);
  xTaskCreate(StatusTask, "Status Task", 4096, NULL, 2, NULL);

  safePrintln("ESP32-C6 FreeRTOS object detection alert system started");
  safePrintln("Hardware: LED on GPIO 2, active buzzer on GPIO 8");
  safePrintln("Serial commands: PERSON_DETECTED, CHAIR_DETECTED, BOTTLE_DETECTED, PHONE_DETECTED, LAPTOP_DETECTED, TEST_ALERT");
}

void loop() {
  vTaskDelay(pdMS_TO_TICKS(1000));
}
