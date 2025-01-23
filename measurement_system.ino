
//PINs
int voltagePin = 35; // Voltage input pin
int currentPin = 34; // Current input pin

// Voltage Variables
float meanVoltage = 0;
float voltageSamples[1000];
float voltageSquaredSum = 0;
float voltageRMS[1000];

// Current Variables
float meanCurrent = 0;
float currentSamples[1000];
float currentSquaredSum = 0;
float currentRMS[1000];

// Power Variables
float apparentPower[1000];
float instantaneousPower;
float activePower[1000];
float powerFactor[1000];

// Time Auxiliary Variable
unsigned long timerAux;   // For sampling every 200µs
int timeToSend = 0;          // Checks if one minute has passed

// Auxiliary Variable for Averaging
int indexToSend = 0;

// Energy Variable
float energyDelivered;

#include <WiFi.h>
#include "ThingSpeak.h"

/********************************************************************************************/
/********************************************************************************************/
//FILL THIS WITH YOUR INFORMATION!!!!!!
const char* ssid = "YOUR_SSID";             // Network SSID (name) 
const char* password = "YOUR_PASSWORD";    // Network password

WiFiClient  client;

unsigned long myChannelNumber = YOUR_CHANNEL_NUMBER;       // ThingSpeak channel number
const char * myWriteAPIKey = "YOUR_API_KEY";              // ThingSpeak API key
/********************************************************************************************/
/********************************************************************************************/

// Timer variables
unsigned long lastTime = 0;
unsigned long publishTime = 1000*60;  // 1 minute publish interval

void setup() {
  // Initialize Serial Communication
  Serial.begin(9600);

  // Set Wi-Fi mode to Station
  WiFi.mode(WIFI_STA);   
  ThingSpeak.begin(client);  // Initialize ThingSpeak
}

void publishToThingSpeak(float voltRMS, float currRMS, float apparentPower, float activePower, float powerFactor, float energy) {
  if ((millis() - lastTime) > publishTime) {  
    // Connect or reconnect to Wi-Fi
    if(WiFi.status() != WL_CONNECTED){
      Serial.print("Attempting to connect");
      while(WiFi.status() != WL_CONNECTED){
        WiFi.begin(ssid, password); 
        delay(5000);     
      } 
      Serial.println("\nConnected.");
    }

    // Set the fields for ThingSpeak channel
    ThingSpeak.setField(1, voltRMS);
    ThingSpeak.setField(2, currRMS);
    ThingSpeak.setField(3, apparentPower);
    ThingSpeak.setField(4, activePower);
    ThingSpeak.setField(5, powerFactor);
    ThingSpeak.setField(6, energy);
    
    // Write data to ThingSpeak. There are up to 8 fields in a channel, allowing you to store up to 8 different
    // pieces of information in a channel.  Here, we write to field 1.
    int x = ThingSpeak.writeFields(myChannelNumber, myWriteAPIKey);

    if(x == 200){
      Serial.println("Channel update successful.");
    } else {
      Serial.println("Problem updating channel. HTTP error code " + String(x));
    }
    lastTime = millis();
  }
}

void loop() {
  meanVoltage = 0;
  meanCurrent = 0;

  // Read and convert 1000 samples (Analog to Digital conversion)
  for(int index = 0; index < 1000; index++) {
    timerAux = micros();

    // Voltage
    voltageSamples[index] = (3.3/4096) * analogRead(voltagePin);
    meanVoltage += voltageSamples[index];

    // Current
    currentSamples[index] = (3.3/4096) * analogRead(currentPin);
    meanCurrent += currentSamples[index];

    // Control the time interval for readings (every 200µs)
    while(micros() - timerAux < 200) {
      // Wait for 200µs before taking a new voltage and current sample
    }
  }

  // Calculate the average of the 1000 samples for voltage and current
  meanVoltage = meanVoltage / 1000;
  meanCurrent = meanCurrent / 1000;  

  // Subtract the mean from each sample to reduce measurement errors
  for(int index = 0; index < 1000; index++) {
    voltageSamples[index] = 2 * 132 * (voltageSamples[index] - meanVoltage); // Voltage adjustment
    currentSamples[index] = 20 * (currentSamples[index] - meanCurrent);     // Current adjustment
  }

  voltageSquaredSum = 0;
  currentSquaredSum = 0;
  instantaneousPower = 0;

  // Sum all the squared values for voltage and current
  for(int i = 0; i < 1000; i++) {
    voltageSquaredSum += (voltageSamples[i] * voltageSamples[i]);
    currentSquaredSum += (currentSamples[i] * currentSamples[i]);
    instantaneousPower += (voltageSamples[i] * currentSamples[i]);
  }

  // Calculate Active Power
  activePower[indexToSend] = instantaneousPower / 1000;
  Serial.println("Active Power: " + String(activePower[indexToSend]));

  // Calculate RMS Voltage
  voltageRMS[indexToSend] = sqrt(voltageSquaredSum / 1000);
  Serial.println("RMS Voltage = " + String(voltageRMS[indexToSend]));

  // Calculate RMS Current
  currentRMS[indexToSend] = sqrt(currentSquaredSum / 1000);
  Serial.println("RMS Current = " + String(currentRMS[indexToSend]));

  // Calculate Apparent Power
  apparentPower[indexToSend] = voltageRMS[indexToSend] * currentRMS[indexToSend];
  Serial.println("Apparent Power: " + String(apparentPower[indexToSend]));

  // Calculate Power Factor
  powerFactor[indexToSend] = activePower[indexToSend] / apparentPower[indexToSend];
  Serial.println("Power Factor: " + String(powerFactor[indexToSend]));

  indexToSend++;

  // Delay for 10 seconds before next batch of 1000 samples
  delay(10000); // 10 seconds

  // Increment the time check variable
  timeToSend += 10;

  // If 1 minute has passed, calculate the averages for all parameters
  if(timeToSend == 60) { // 1 minute passed
    for(int index = 0; index < indexToSend; index++) {        
      // Sum all the values
      voltageRMS[0] += voltageRMS[index + 1];
      currentRMS[0] += currentRMS[index + 1];
      apparentPower[0] += apparentPower[index + 1];
      activePower[0] += activePower[index + 1];
      powerFactor[0] += powerFactor[index + 1];
    } 

    /**************************/
    /* Calculate the averages */
    /**************************/
    voltageRMS[0] = voltageRMS[0] / indexToSend;
    Serial.println("Voltage RMS[0]: " + String(voltageRMS[0]));
    
    currentRMS[0] = currentRMS[0] / indexToSend;
    Serial.println("Current RMS[0]: " + String(currentRMS[0]));
    
    apparentPower[0] = apparentPower[0] / indexToSend;
    Serial.println("Apparent Power[0]: " + String(apparentPower[0]));
    
    powerFactor[0] = powerFactor[0] / indexToSend;
    Serial.println("Power Factor[0]: " + String(powerFactor[0]));
    
    activePower[0] = activePower[0] / indexToSend;
    Serial.println("Active Power[0]: " + String(activePower[0]));
    
    // Calculate Energy Delivered
    energyDelivered = (activePower[0] / indexToSend) / 60000;
    Serial.println("Energy Delivered: " + String(energyDelivered));

    // Send data to ThingSpeak
    publishToThingSpeak(voltageRMS[0], currentRMS[0], apparentPower[0], activePower[0], powerFactor[0], energyDelivered); 

    // Reset auxiliary variables
    timeToSend = 0;
    indexToSend = 0;
  }
}
