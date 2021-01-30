#include <Arduino.h>

#include "DHT.h" // https://github.com/adafruit/DHT-sensor-library

// =========================================================================================
// Definitions
// =========================================================================================

#define DHTTYPE DHT11 // DHT 11

#define I_VCC 5.0f // supply voltage of the current sensor
#define V_VCC 5.0f // supply voltage of the voltage sensor

#define NUMBER_OF_SAMPLES 1480

#define ADC_BITS 10
#define ADC_COUNTS (1 << ADC_BITS)

#define CURRENT_CONVERSION_CONSTANT (I_VCC / ADC_COUNTS) * 10.0f
#define VOLTAGE_CONVERSION_CONSTANT (V_VCC / ADC_COUNTS) * 480.0f

#define INPUT_PIN_VOLTAGE A0
#define INPUT_PIN_TEMP_UMI A2
#define INPUT_PIN_CURRENT A5

// =========================================================================================

DHT dht(INPUT_PIN_TEMP_UMI, DHTTYPE);

float offsetI = 1011.90f; //Low-pass filter output [ADC_COUNTS >> 1]
float offsetV = 507.000f; //Low-pass filter output [ADC_COUNTS >> 1]

float sumI = 0.0f, sumV = 0.0f, sumP = 0.0f;

float filteredI, filteredV;

float apparent_power = 66.71f;
float real_power = 37.25f;
float voltage = 220.0f;
float current = 0.30f;
float power_factor = real_power / apparent_power;

float humidity = 0.0f;
float temperature = 0.0f;

unsigned int n = 0;

int currentWaveFormComponent = 0;
int voltageWaveFormComponent = 0;

// =========================================================================================

void process_information()
{

  sumP = 0;
  sumI = 0;
  sumV = 0;

  for (n = 0; n < NUMBER_OF_SAMPLES; n++)
  {
    currentWaveFormComponent = analogRead(INPUT_PIN_CURRENT);
    voltageWaveFormComponent = analogRead(INPUT_PIN_VOLTAGE);

    offsetI = (offsetI + (currentWaveFormComponent - offsetI) / 1024);
    offsetV = offsetV + ((voltageWaveFormComponent - offsetV) / 1024);

    filteredI = currentWaveFormComponent - offsetI;
    filteredV = voltageWaveFormComponent - offsetV;

    sumI += (filteredI * filteredI);
    sumV += (filteredV * filteredV);
    sumP += (filteredI * filteredV);
  }

  real_power = VOLTAGE_CONVERSION_CONSTANT * CURRENT_CONVERSION_CONSTANT * (sumP / NUMBER_OF_SAMPLES);
  voltage = (VOLTAGE_CONVERSION_CONSTANT * sqrt(sumV / NUMBER_OF_SAMPLES));
  current = (CURRENT_CONVERSION_CONSTANT * sqrt(sumI / NUMBER_OF_SAMPLES));
  apparent_power = voltage * current;
  power_factor = (apparent_power == 0) ? 0.0f : (real_power / apparent_power);

  humidity = dht.readHumidity();
  temperature = dht.readTemperature();
}

void send_information()
{
  Serial.print("{\"voltage\":");
  Serial.print(voltage);
  Serial.print(",\"current\":");
  Serial.print(current);
  Serial.print(",\"real_power\":");
  Serial.print(real_power);
  Serial.print(",\"apparent_power\":");
  Serial.print(apparent_power);
  Serial.print(",\"power_factor\":");
  Serial.print(power_factor);
  Serial.print(",\"humidity\":");
  Serial.print(humidity);
  Serial.print(",\"temperature\":");
  Serial.print(temperature);
  Serial.println("}");
}

void setup()
{
  Serial.begin(115200);
  dht.begin();
}

void loop()
{
  process_information();
  send_information();
  delay(1000);
}
