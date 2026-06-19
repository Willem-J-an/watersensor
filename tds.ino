#include "tds.h"
#include "Zigbee.h" // IWYU pragma: keep

#define TDS1_ENDPOINT_NUMBER 1
#define TDS2_ENDPOINT_NUMBER 2
#define TDS3_ENDPOINT_NUMBER 3
#define FLOW_SENSOR_ENDPOINT_NUMBER 4
#define TEMP_SENSOR_ENDPOINT_NUMBER 5

uint8_t button = BOOT_PIN;
ZigbeeAnalog zbTds1 = ZigbeeAnalog(TDS1_ENDPOINT_NUMBER);
ZigbeeAnalog zbTds2 = ZigbeeAnalog(TDS2_ENDPOINT_NUMBER);
ZigbeeAnalog zbTds3 = ZigbeeAnalog(TDS3_ENDPOINT_NUMBER);
ZigbeeAnalog zbTds[3] = { zbTds1, zbTds2, zbTds3 };
ZigbeeAnalog zbFlowSensor = ZigbeeAnalog(FLOW_SENSOR_ENDPOINT_NUMBER);
ZigbeeTempSensor zbTempSensor = ZigbeeTempSensor(TEMP_SENSOR_ENDPOINT_NUMBER);
float prevFlowRate = 0.0;

void setup() {
  Serial.begin(115200);
  Serial.println("Starting sensor...");
  pinMode(button, INPUT_PULLUP);
  tdsSensorSetup();
  flowRateSetup();

  zbTds1.setManufacturerAndModel("WillemJan", "WaterSensor");
  for (int i = 0; i < 3; i++) {
    zbTds[i].addAnalogInput();
    char desc[12];
    sprintf(desc, "TDS Value %d", i);
    zbTds[i].setAnalogInputDescription(desc);
    zbTds[i].setAnalogInputResolution(1);
    zbTds[i].setAnalogInputMinMax(0.0, 2000.0);
    zbTds[i].setAnalogInputApplication(ESP_ZB_ZCL_AI_APP_TYPE_PPM);
    Zigbee.addEndpoint(&zbTds[i]);
  }
  zbFlowSensor.addAnalogInput();
  zbFlowSensor.setAnalogInputDescription("Water flow");
  zbFlowSensor.setAnalogInputResolution(0.01);
  zbFlowSensor.setAnalogInputMinMax(0.0, 10.0);
  zbFlowSensor.setAnalogInputApplication(ESP_ZB_ZCL_AI_APP_TYPE_FLOW);
  Zigbee.addEndpoint(&zbFlowSensor);

  zbTempSensor.setMinMaxValue(0, 100);
  zbTempSensor.setDefaultValue(10.0);
  zbTempSensor.setTolerance(0.1);
  Zigbee.addEndpoint(&zbTempSensor);

  Serial.println("Starting Zigbee...");
  if (!Zigbee.begin(ZIGBEE_END_DEVICE)) {
    Serial.println("Zigbee failed to start!");
    Serial.println("Rebooting...");
    ESP.restart();
  } else {
    Serial.println("Zigbee started successfully!");
  }
  Serial.println("Connecting to network...");
  while (!Zigbee.connected()) {
    Serial.print(".");
    delay(100);
  }
  Serial.println("Connected!");
  for (int i = 0; i < 3; i++) {
    zbTds[i].setAnalogInputReporting(0, 3600, 1);
  }
  zbFlowSensor.setAnalogInputReporting(0, 3600, 0.01);
  zbTempSensor.setReporting(0, 3600, 1);
  Serial.println("Sensor started!");
}

void loop() {
  static uint32_t flowTimeCounter = 0;
  static uint32_t timeCounter = 0;
  static uint8_t interval = 100;
  if (!(flowTimeCounter++ % 2000) ) { // 2 sec
    float flowRate = getFlowRate();
    if (flowRate != prevFlowRate) {
      Serial.printf("Flowrate: %.2f L/min\n", flowRate);
      zbFlowSensor.setAnalogInput(flowRate);
      prevFlowRate = flowRate;
    }
  }

  if (!(timeCounter++ % 60000)) { // 60 sec
    float temp = getTemp();
    Serial.printf("Temp: %.2f °C\n", temp);
    zbTempSensor.setTemperature(temp);
    TdsData tdsData = tdsSensorRead(temp);

    for (int i = 0; i < 3; i++) {
      Serial.printf("Updating TDS %d to %d ppm\n", i, tdsData.values[i]);
    }

    for (int i = 0; i < 3; i++) {
      zbTds[i].setAnalogInput(tdsData.values[i]);
    }
  }

  if (digitalRead(button) == LOW) {
    delay(100);
    int startTime = millis();
    while (digitalRead(button) == LOW) {
      delay(50);
      if ((millis() - startTime) > 3000) {
        Serial.println("Resetting Zigbee to factory and rebooting in 1s.");
        delay(1000);
        Zigbee.factoryReset();
      }
    }
  }

  delay(1);
}
