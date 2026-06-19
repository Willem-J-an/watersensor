#define S1_TDS 0 // orange
#define S1_POWER 20
#define S1_TEMP -1 // brown -- unused
#define S1_CALIBRATION 1
#define S2_TDS 2 // white
#define S2_POWER 19
#define S2_TEMP -1
#define S2_CALIBRATION 1
#define S3_TDS 3 // grey
#define S3_POWER 18
#define S3_CALIBRATION 1
#define S3_FLOWRATE 21 // yellow


static const float VREF = 1.1;
static const float ADC_RES = 4096.0;
static const float TEMP_COEFFICIENT = 0.02;
static const float STD_TEMP = 25.0;

volatile long flowPulseCount = 0;
unsigned long oldTime = 0;

float flowRate = 0.0;

void IRAM_ATTR flowPulseCounter() {
  flowPulseCount++;
}

struct Pins {
  int tds;
  int power;
  int temp;
  float calibration;
};

Pins modulePins[3] = {
  {S1_TDS, S1_POWER, S1_TEMP, S1_CALIBRATION},
  {S2_TDS, S2_POWER, S2_TEMP, S2_CALIBRATION},
  {S3_TDS, S3_POWER, -1, S3_CALIBRATION},
};

void tdsSensorSetup() {
  for (int i = 0; i < 3; i++) {
    pinMode(modulePins[i].power, OUTPUT);
    // analogSetPinAttenuation(modulePins[i].tds, ADC_2_5db);
  }
  analogReadResolution(12);
}

void flowRateSetup() {
  pinMode(S3_FLOWRATE, INPUT);
  attachInterrupt(digitalPinToInterrupt(S3_FLOWRATE), flowPulseCounter, FALLING);
}

TdsData tdsSensorRead(float temp) {
  TdsData result;
  for (int i = 0; i < 3; i++) {
    digitalWrite(modulePins[i].power, HIGH);
    delay(500);
    
    long sumMv = 0;
    for(int ii = 0; ii < 30; ii++) {
      sumMv += analogReadMilliVolts(modulePins[i].tds);
      delay(10);
    }
    
    digitalWrite(modulePins[i].power, LOW);
    delay(500);

    float voltage = (sumMv / 30.0) / 1000.0;

    float compensationFactor = 1.0 + TEMP_COEFFICIENT * (temp - STD_TEMP);
    float compensationVoltage = voltage / compensationFactor;
    float correctedVoltage = compensationVoltage * (5.0/3.3);
    result.values[i] = (133.42 * pow(correctedVoltage, 3)
                      - 255.86 * pow(correctedVoltage, 2)
                      + 857.39 * correctedVoltage) * 0.5 * modulePins[i].calibration;
  }
  return result;
}

float getFlowRate(){
  noInterrupts();
  unsigned long pulses = flowPulseCount;
  flowPulseCount = 0;
  unsigned long currentTime = millis();
  unsigned long durationMs = currentTime - oldTime;
  oldTime = currentTime;
  interrupts();

  float timeElapsedSeconds = durationMs / 1000.0;
  if (timeElapsedSeconds <= 0 || pulses == 0) return 0.0;
  float frequency = (float)pulses / timeElapsedSeconds;
  flowRate = (frequency / 36.0) * 1.03;

  return round(flowRate *100.0) / 100.0;
}
