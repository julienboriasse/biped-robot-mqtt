#include <Arduino.h>
#include "secrets.h"
#include "WiFi.h"

const char *ssid = WIFI_SSID;
const char *password = WIFI_PASSWORD;

#define NUMBER_OF_MOTORS 6
#define MOTOR_PWM_DUTY_CYCLE_MINIMUM 3.5
#define MOTOR_PWM_DUTY_CYCLE_MAXIMUM 11.5

String translateEncryptionType(wifi_auth_mode_t encryptionType)
{

  switch (encryptionType)
  {
  case (WIFI_AUTH_OPEN):
    return "Open";
  case (WIFI_AUTH_WEP):
    return "WEP";
  case (WIFI_AUTH_WPA_PSK):
    return "WPA_PSK";
  case (WIFI_AUTH_WPA2_PSK):
    return "WPA2_PSK";
  case (WIFI_AUTH_WPA_WPA2_PSK):
    return "WPA_WPA2_PSK";
  case (WIFI_AUTH_WPA2_ENTERPRISE):
    return "WPA2_ENTERPRISE";
  default:  
    return "UNKNOWN";
  }
  return "UNKNOWN";
}

void scanNetworks()
{

  int numberOfNetworks = WiFi.scanNetworks();

  Serial.print("Number of networks found: ");
  Serial.println(numberOfNetworks);

  for (int i = 0; i < numberOfNetworks; i++)
  {

    Serial.print("Network name: ");
    Serial.println(WiFi.SSID(i));

    Serial.print("Signal strength: ");
    Serial.println(WiFi.RSSI(i));

    Serial.print("MAC address: ");
    Serial.println(WiFi.BSSIDstr(i));

    Serial.print("Encryption type: ");
    String encryptionTypeDescription = translateEncryptionType(WiFi.encryptionType(i));
    Serial.println(encryptionTypeDescription);
    Serial.println("-----------------------");
  }
}

void connectToNetwork()
{
  WiFi.begin(ssid, password);

  while (WiFi.status() != WL_CONNECTED)
  {
    delay(1000);
    Serial.println("Establishing connection to WiFi..");
  }

  Serial.println("Connected to network");
}

int dutyCycle;

const int LEDPin = BUILTIN_LED; /* GPIO16 */
const int motorPin[NUMBER_OF_MOTORS] = {13, 12, 27, 33, 15, 32};

/* Setting PWM Properties */
const int PWMFreq = 50; /* 50 Hz or 20ms for HS-422 servo motors */
const int PWMResolution = 10;
const int MIN_DUTY_CYCLE = 3;
const int MAX_DUTY_CYCLE = 12;

/*
  Function to set motor PWM duty cycle in percent
  limiting values between MOTOR_PWM_DUTY_CYCLE_MINIMUM (3.5%) and MOTOR_PWM_DUTY_CYCLE_MAXIMUM (11.5%)
*/
void motorWrite(uint8_t chan, float duty)
{
  /* Saturate duty cycle value */
  duty = duty < MOTOR_PWM_DUTY_CYCLE_MINIMUM ? MOTOR_PWM_DUTY_CYCLE_MINIMUM : duty;
  duty = duty > MOTOR_PWM_DUTY_CYCLE_MAXIMUM ? MOTOR_PWM_DUTY_CYCLE_MAXIMUM : duty;
  /* scale value from 0-100 to 0-255 */
  duty = duty * 255 / 100;
  ledcWrite(chan, (uint32_t)duty);
}

void setup()
{
  Serial.begin(115200);

  scanNetworks();
  connectToNetwork();

  Serial.println(WiFi.macAddress());
  Serial.println(WiFi.localIP());
  
  /* Initialize PWM channel and pin for each motor */
  for (int PWMChannel = 0; PWMChannel < NUMBER_OF_MOTORS; PWMChannel++)
  {
    ledcSetup(PWMChannel, PWMFreq, PWMResolution);
    ledcAttachPin(motorPin[PWMChannel], PWMChannel);
  }
}

void loop()
{

  for (int PWMChannel = 0; PWMChannel < NUMBER_OF_MOTORS; PWMChannel++)
    motorWrite(PWMChannel, 3.5+PWMChannel);

  while (1);

  /* Increasing the motors positions with PWM */
  for (dutyCycle = 0; dutyCycle <= MAX_DUTY_CYCLE; dutyCycle++)
  {
    for (int PWMChannel = 0; PWMChannel < NUMBER_OF_MOTORS; PWMChannel++)
      motorWrite(PWMChannel, dutyCycle);
    delay(3);
  }
  /* Decreasing the motors positions with PWM */
  for (dutyCycle = MAX_DUTY_CYCLE; dutyCycle >= 0; dutyCycle--)
  {
    for (int PWMChannel = 0; PWMChannel < NUMBER_OF_MOTORS; PWMChannel++)
      motorWrite(PWMChannel, dutyCycle);
    delay(3);
  }
}