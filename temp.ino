#define S3_TEMP 5 // green
#define R_FIXED 120000.0
#define V_SOURCE 4.83
#define V_REF 3.28
#define ADC_MAX 4095.0

#define R_NOMINAL 50000.0
#define T_NOMINAL 25.0
#define B_COEFFICIENT 3942.0
#define TEMP_CALIBRATION 1.24

float getTemp() {
  float averageADC = 0;
  for (int i = 0; i < 10; i++) {
    averageADC += analogRead(S3_TEMP);
    delay(10);
  }
  averageADC /= 10;
  float vOut = (averageADC / ADC_MAX) * V_REF *TEMP_CALIBRATION;
  float rNTC = (vOut * R_FIXED) / (V_SOURCE - vOut);

  float temperature;
  temperature = rNTC / R_NOMINAL;
  temperature = log(temperature);
  temperature /= B_COEFFICIENT;
  temperature += 1.0 / (T_NOMINAL + 273.15);
  temperature = 1.0 / temperature;
  temperature -= 273.15;
  Serial.printf("Voltage: %.2f V | Resistance: %.2f kOhm\n", vOut, rNTC / 1000);
  return round(temperature *100.0) / 100.0;
}
