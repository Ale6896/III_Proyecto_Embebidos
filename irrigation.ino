#include <EEPROM.h>
#include <DHT.h>
#include <LiquidCrystal.h>

#define DHTPIN 2      // Pin where the DHT22 sensor is connected
#define DHTTYPE DHT22 // DHT sensor type
#define LED_PIN 13

DHT dht(DHTPIN, DHTTYPE);
LiquidCrystal lcd(3, 4, 5, 6, 7, 8, 9, 10, 11, 12);

// Define the starting memory address for storing data
const int DATA_START_ADDRESS = 18; // Skip the first two addresses for the pointers

// Define the total number of memory addresses available for data storage
const int MEMORY_SIZE = 1023; // Total EEPROM size is 1024 bytes, but we leave 2 bytes for the pointers

unsigned long previousTemperatureMillis = 0;
unsigned long previousStoreMillis = 0;
const unsigned long temperatureInterval = 1000; // Read temperature every 1 seconds
const unsigned long storeInterval = 10000;      // Store data every 2 seconds
float treshold = 24.0;

bool storeTemperatureFlag = false;
float temperatureValue;
int counter = 0;

void setup() {
  // Initialize the serial communication
  Serial.begin(9600);

  lcd.begin(16, 2);
  
  // Initialize the DHT sensor
  dht.begin();

  clearMemory();

  pinMode(LED_PIN, OUTPUT);

  // Read the current pointer values from the first two memory addresses
  int nextAddress = EEPROM.read(0);
  int endAddress = EEPROM.read(1);

  // Check if the stored addresses are out of bounds or uninitialized
  if (nextAddress < DATA_START_ADDRESS || nextAddress >= MEMORY_SIZE) {
    // Initialize the next address pointer to the starting address for data storage
    nextAddress = DATA_START_ADDRESS;
    EEPROM.write(0, nextAddress);
  }

  if (endAddress < DATA_START_ADDRESS || endAddress >= MEMORY_SIZE) {
    // Initialize the end address pointer to the starting address for data storage
    endAddress = DATA_START_ADDRESS;
    EEPROM.write(1, endAddress);
  }

  storeData("20230615224532",true);

}



void loop() {

  if (Serial.available() > 0) {
    Serial.println("Starting...");
    // Read the incoming data from the serial port
    String msg = Serial.readString();
    char command = msg[0];
    String b = msg.substring(1);


    // Update date and time
    if (command == 'D'){ 
      // Store the received data (datetime)
      storeData(b, true);
    }

    // Ask for saved data
    else if (command == 'A'){

      int nextAddress = EEPROM.read(0);
      // Print the stored data
      // Calculate the length of the data
      int dataLength = nextAddress - 2;

      // Create a buffer to store the data
      uint8_t buffer[dataLength];
      // Read data from EEPROM and store it in the buffer
      for (int i = 2; i < nextAddress; i++) {
        buffer[i - 2] = EEPROM.read(i);
        // You can perform any additional processing here
      }

      Serial.write(buffer, nextAddress);
      
      
      //clearMemory();
    }

    // Update Treshold
    else if (command == 'T'){
      // Falta revizar si funciona

      String T = b;
      treshold = T.toFloat();
      Serial.println(msg);
      lcd.clear();
      lcd.print(treshold);
    }
   
  }
  // Read temperature from the DHT22 sensor every 10 seconds
  unsigned long currentMillis = millis();

  if (currentMillis - previousTemperatureMillis >= temperatureInterval) {
    readTemperature();
    previousTemperatureMillis = currentMillis;    
  }

     // Store the temperature every 60 seconds
  if (currentMillis - previousStoreMillis >= storeInterval) {
    previousStoreMillis = currentMillis;
    storeTemperatureFlag = true;
    counter += 1;
    storeTemperature(temperatureValue, counter); 
  }

  activateIrrigation(temperatureValue);

}

void readTemperature(){
  
  float temperature = dht.readTemperature();

    lcd.clear();
    lcd.print("Temp: ");
    lcd.print(temperature);
    lcd.print(" *C");
    lcd.setCursor(0,1);
    lcd.print("Tresh: ");
    lcd.print(treshold);
    lcd.print(" *C");

    // Check if the temperature reading is valid
    if (!isnan(temperature)) {
      temperatureValue = temperature;
    }

}


void clearMemory(){
  for (int i = 0; i < EEPROM.length();i++){
    EEPROM.write(i,0);
  }
  EEPROM.write(0,DATA_START_ADDRESS);
  EEPROM.write(1,DATA_START_ADDRESS);
}

// Function to handle storing the received data (either datetime or temperature)
void storeData(const String& receivedData, bool isDateTime) {
  // Get the current next address and end address from EEPROM
  int nextAddress = EEPROM.read(0);
  int endAddress = EEPROM.read(1);

  // Store the received data at the next address
  for (int i = 0; i < receivedData.length(); i++) {
    if (isDateTime){
      EEPROM.write(i+2, receivedData[i]);
    }
    else{
      EEPROM.write(nextAddress, receivedData[i]);
      nextAddress++;
    }
    
  }

  
}

void storeTemperature(float temperatureValue, int counter){
    String temperatureString = 'T' + String(temperatureValue*10, 0); // 2 decimal places

    // Store the temperature data
    storeData(temperatureString, false);

    // Reset the flag
    storeTemperatureFlag = false;

    // Store counter
    storeData('C' + String(counter), false);
}

void activateIrrigation(float temperatureValue){
  if (temperatureValue >= treshold){
    digitalWrite(LED_PIN, HIGH);
  }
  else{
    digitalWrite(LED_PIN, LOW);
  }
}
