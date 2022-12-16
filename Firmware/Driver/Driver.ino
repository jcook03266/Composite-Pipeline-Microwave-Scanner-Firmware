#include <Adafruit_MotorShield.h>
#include <Wire.h>
#include <AFMotor.h>
#include "SoftwareSerial.h"
#include <LiquidCrystal.h>

//Switch communication
//RF Switchboard Addressing Mode Truth table:
//Control Input - Signal Path State (RFCOM to:)
//ABC
//000 RF1
//100 RF2
//010 RF3
//110 RF4
//001 RF5
//101 RF6
//011 RF7
//111 RF8

int addressingModes[8][3] = {
  { LOW, LOW, LOW },
  { HIGH, LOW, LOW },
  { LOW, HIGH, LOW },
  { HIGH, HIGH, LOW },
  { LOW, LOW, HIGH },
  { HIGH, LOW, HIGH },
  { LOW, HIGH, HIGH },
  { HIGH, HIGH, HIGH },
};
int usedAddressingModes[6][3] {
  { LOW, LOW, LOW }, // Port 1 
  { HIGH, LOW, LOW }, // Port 2
  { LOW, HIGH, LOW }, // Port 3
  { HIGH, LOW, HIGH }, // Port 6
  { LOW, HIGH, HIGH }, // Port 7
};
int controlInputPins[] = { 4, 5, 6 };  // A,B,C

//Power Meter Reading
int powerMeterSensorValue = 0;
float inputVoltage = 5.0;   //The DC voltage supplied to the rest of the circuit
float outputVoltage = 0.0;  //The DC voltage output by the power meter
int powerMeterPin = A0;

//Cycle Control
int antennaCount = 6;
int cycleDelay = 500;
int currentReadingBatch = 0;

//BT Communication
SoftwareSerial BTConnection(2, 3);  //Create a serial connection with TX and RX on these pins
#define BUFFER_SIZE 64              //This will prevent buffer overruns.
char inData[BUFFER_SIZE];           //This is a character buffer where the data sent by the python script will go.
char inChar = -1;                   //Initialie the first character as nothing
int count = 0;                      //This is the number of lines sent in from the python script
int i = 0;                          //Looping variable
int baudRate = 9600;
int communicationDebounce = 1000;  //Pause BT comms for 1 second(s)

//Movement State Management
int horizontalOffset = 0; //Total movement horizontally [mm]

//LCD Display
const int rs = 13, en = 12, d4 = 11, d5 = 10, d6 = 9, d7 = 8;
LiquidCrystal lcd = LiquidCrystal(rs,en,d4,d5,d6,d7);
const int rows = 2;
const int columns = 16;

// Data transmission
const int totalDataPoints = 12;
int sampleSize = 10;
int scanCount = 0; // Count up to 10 and then go back to 0 after every batch of scans
int dataFields = 4; // Port number, phi, voltage, z offset
int batchedData [12][4] = {}; // 5 scans * 6 ports = 30 total data points, and each data point has the port number and voltage

// Lower level hardware delegate classes and custom data types
struct RFScan {
  int id;
  int port;
  float voltage;
  int z_Offset;

  RFScan(int id = 0, int port = 1, float voltage = 0, int z_Offset=0):
    id(id), 
    port(port), 
    voltage(voltage),
    z_Offset(z_Offset){}

  int getPhi() {
    switch(port) {
   case 1  :
      return 0;
   case 2 :
      return 60;
  case 3 :
      return 120;
  case 6 :
      return 180;
  case 7 :
      return 240;
  case 8 :
      return 300;
  
   default :
      return 0;
}
  }

  float getVoltageInMillivolts() {
    return voltage * 1000;
  }

  String getFormattedScanData(bool longFormat) {
    String formattedData = "";
    if (longFormat) {
      formattedData += "Power Meter Voltage: ";
      formattedData +=  String(voltage);
      formattedData += "[mV]\nRF Port: ";
      formattedData +=  String(port);
    }
    else {
      formattedData += "Voltage: ";
      formattedData +=  String(voltage);
      formattedData += " [mV]\nPort: ";
      formattedData +=  String(port);
    }
    formattedData += "\n";

    return formattedData;
  }

  /// Prints out the formatted contents of the struct to the console
  void displayContents(bool longFormat) {
    String formattedString = getFormattedScanData(longFormat);

    Serial.println(formattedString);
    lcd.print(formattedString);
  }
};

// Higher level managers and controllers
/// Controls the motor speed by communicating with the detection manager to constantly get feedback about the current scan conditions
class MotorController {
  public:
  int id;
  bool wasLoaded;
  int total_Z_Offset = 0;
  int smallStepSize_Z_Offset = 5; // In [cm]
  int largeStepSize_Z_Offset = 10; // In [cm]

  Adafruit_MotorShield AFMS = Adafruit_MotorShield();
  Adafruit_StepperMotor *myMotor = AFMS.getStepper(200, 2);
  
 MotorController(int id = 0):
  id(id) {}

  void setup() {
    if (!wasLoaded) {
    if (!AFMS.begin()) {         
    Serial.println("Could not find Motor Shield. Check wiring.");
    }

    Serial.println("Motor Shield found.");
    myMotor->setSpeed(10); // 50RPM
    }

    wasLoaded = true;
  }

  void start() {
    baseMovement();
  }

  void stop() {
    noStep();
  }

  void baseMovement() {
      interleaveStep();

      total_Z_Offset += largeStepSize_Z_Offset;
  }

  void slowedMovement() {
      microStep();

      total_Z_Offset += smallStepSize_Z_Offset;
  }

  // Movement routines with different step sizes
  void doubleStep() {
    String stepSizeDescription = "Step: Double";

    displayStepSizeInformation(stepSizeDescription);

    myMotor->step(150, FORWARD, DOUBLE);
    //myMotor->step(150, BACKWARD, DOUBLE);
  }

  void interleaveStep() {
    String stepSizeDescription = "Step: Interleave";

    displayStepSizeInformation(stepSizeDescription);

    myMotor->step(200, FORWARD, INTERLEAVE);
    //myMotor->step(200, BACKWARD, INTERLEAVE);
  }

  void microStep() {
    String stepSizeDescription = "Step: Micro";

    displayStepSizeInformation(stepSizeDescription);

    myMotor->step(100, FORWARD, MICROSTEP);
    //myMotor->step(100, BACKWARD, MICROSTEP);
  }

  void noStep() {
    String stepSizeDescription = "Step: None";

    displayStepSizeInformation(stepSizeDescription);

    myMotor->step(0, FORWARD, MICROSTEP);
  }

  void displayStepSizeInformation(String information) {
    Serial.println(information);
  }
};

/// Reports to its respective subscribers the current condition of the pipe via communication with the scan controller 
class DetectionManager {
public:
  int id;
  float errorMargin = 0.01; // Error correction (% the scan can be off by)
  MotorController motorController = MotorController(0);
  bool wasLoaded = false;
  bool referenceDataLoaded = false;
  int alertBuzzerPin = 3; //Pin of the buzzer responsible for alerting the user when a defect is found
  int defectsDetected = 0; 
  RFScan referenceScans[6] = {};
  bool defectDetected = false;
  int defectAlertDuration = 1000; // 1 Second

 DetectionManager(int id = 0):
  id(id) {}

  void setup() {
    if (!wasLoaded) {
    defectsDetected = 0;
    defectDetected = false;

    motorController.setup();
    }
    silenceAlarm();
    wasLoaded = true;
  }

  int** getBatchedReferenceData() {
    int** batchedReferenceData = new int*[2];

    for(int i = 0; i < 6; i++) {
      RFScan scan = referenceScans[i];

      int portActualIndex = scan.port;
      int avgVoltage =  scan.voltage;
      
      batchedReferenceData[i][0] = portActualIndex;
      batchedReferenceData[i][1] = avgVoltage;
    }

    return batchedReferenceData;
  }

  void averageReferencePoint(int index, RFScan newScan) {
      if (!defectDetected) {
        RFScan referenceScan = referenceScans[index];
        float totalVoltage = referenceScan.voltage + newScan.voltage;

        float avgVoltage = totalVoltage / 2;
        referenceScan.voltage = avgVoltage;
        
        referenceScans[index] = referenceScan;

        // Inform the system that a new reference point has been calculated 
        Serial.println("");
        Serial.print("New Reference Data Point Calculated at Port ");
        Serial.print(index);
        Serial.println("");
        Serial.println(referenceScans[index].voltage);
      }
  }

  void classify(RFScan scanBatch[6]) {
    defectDetected = false;
    
    for (i = 0; i < 6; i++) {
      RFScan scan = scanBatch[i];
      RFScan reference = referenceScans[i];

      float acceptableErrorMargin = reference.voltage * errorMargin;
      float lowerBound = reference.voltage - acceptableErrorMargin;
      float upperBound = reference.voltage + acceptableErrorMargin;
      bool condition = scan.voltage >= lowerBound && scan.voltage <= upperBound; 

      // If the voltage is out of bounds trigger detection protocol
      if (!condition) {
      Serial.println("Defect Detected at");
      Serial.print("Port ");
      Serial.print(scan.port);
      Serial.println("");
        
      defectsDetected += 1;
      defectDetected = true;

      displayDefectDetectionInstructions(scan.port);
      }
    }

    if (defectDetected) { activateAlarm(); }
    else { silenceAlarm(); }

    defectDetected ? motorController.slowedMovement() : motorController.baseMovement();
  }

  void silenceAlarm() {
    digitalWrite(alertBuzzerPin, LOW);
  }

  //3 Tier, 3 repetitions Asynchronous Sequential Buzz (9 second duration)
  void activateAlarm() {
  for (int i = 0; i < 3; i++) {
    digitalWrite(alertBuzzerPin, HIGH);
    delay(defectAlertDuration/2);
    digitalWrite(alertBuzzerPin, LOW);
    delay(defectAlertDuration/2);
  }
}

//Displayed when a defect is detected
void displayDefectDetectionInstructions(int port) {
  lcd.clear();
  lcd.noBlink();

  lcd.setCursor(0,0);
  lcd.print("Defect @ P: " + String(port));
  lcd.setCursor(0,1);
  lcd.print("Total Defects: " + String(defectsDetected));

  delay(defectAlertDuration);
}
};

// Dependency injections
DetectionManager detectionManager = DetectionManager();

// RF Scans
struct RFScan lastScanBatch[6];

void setup() {
  detectionManager.setup();

  // Silence the detection buzzer
  detectionManager.silenceAlarm();

  //Set rate of communication
  Serial.begin(baudRate);

  initializeLCD();

  setPowerMeterPinIOStates();
}

void initializeLCD() {
   //Set up the LCD's number of columns and rows
  lcd.begin(columns,rows);
  scanStartLCD();
  Serial.println("Started");
}

void setPowerMeterPinIOStates() {
  pinMode(powerMeterPin, INPUT); //Set the power meter pin as input

  //Designate control input pins as output because they are used to control the switch array
  for (int i = 0; i < sizeof(controlInputPins); i++) {
    pinMode(controlInputPins[i], OUTPUT);
  }
}

void loop() {
  runDataAcquisition();
}

void runDataAcquisition() {
  //Reset the reading batch counter when it reaches 100 to prevent overflow
  if (currentReadingBatch >= 100) {
     currentReadingBatch = 0;
  }

  for(int i = 0; i < antennaCount; i++) {
    // Port selection
    int *currAddressingMode = usedAddressingModes[i];
    // Port tracking
    int portActualIndex = i >= 3 ? (i + 1) + 2 : i + 1 ;
    int portZeroIndex = i;
    // Voltage Reading
    int totalVoltage = 0;
    double avgVoltage = 0.0;
    // Debugging
    bool displayIndividualScans = false;    

    digitalWrite(controlInputPins[0], currAddressingMode[0]);
    digitalWrite(controlInputPins[1], currAddressingMode[1]);
    digitalWrite(controlInputPins[2], currAddressingMode[2]);

    // Print out header
    if (i == 0) {
      currentReadingBatch += 1;

      Serial.println("");
      Serial.print("Reading Batch #");
      Serial.print(currentReadingBatch);
      Serial.print(":");
      Serial.println("");

      // Print out header
      Serial.print(sampleSize);
      Serial.print(" Sample | Scan averages");
      Serial.println("");
    }  
    
    for(int j = 0; j < sampleSize; j++) {
      powerMeterSensorValue = analogRead(powerMeterPin);
      outputVoltage = getVoltageFrom(powerMeterSensorValue);
      totalVoltage += outputVoltage;

      // Print out header
      if (displayIndividualScans) {
      Serial.print("Scan Sample #");
      Serial.print(j);
      Serial.println("");
      RFScan currentScan = RFScan(0, portActualIndex,
      outputVoltage);
  
      currentScan.displayContents(false); 
      }
    }

    avgVoltage = totalVoltage / sampleSize;

    RFScan avgScan = RFScan(0, 
    portActualIndex, 
    avgVoltage, 
    detectionManager.motorController.total_Z_Offset);

    if (!detectionManager.referenceDataLoaded) {
      
    detectionManager.referenceScans[i] = avgScan;

    Serial.println("Reference Data Point");
    Serial.println(detectionManager.referenceScans[i].voltage);

    // Fill up the base reference data for the pipe
    if (i == 5) { detectionManager.referenceDataLoaded = true; }
    }
    else {
      // Start averaging the reference points across the normal parts of the pipe after loading up the initial reference data
     // if (detectionManager.referenceDataLoaded == true) { detectionManager.averageReferencePoint(i, avgScan); }      
    }

    avgScan.displayContents(true);

    lastScanBatch[i] = avgScan; 
    displayScanInformation(avgVoltage, portActualIndex, currentReadingBatch); 

    batchedData[scanCount][0] = avgScan.port;
    batchedData[scanCount][1] = avgScan.getPhi();
    batchedData[scanCount][2] = avgScan.voltage;
    batchedData[scanCount][3] = avgScan.z_Offset;

    scanCount += 1;
    if (scanCount == totalDataPoints) {
      Serial.println("Batched Data Incoming");

      // int** batchedReferenceData = {};
      // batchedReferenceData = detectionManager.getBatchedReferenceData();

      // // // Provide reference data
      // // for (int i = 0; i < 6; i++) {
      // //   Serial.print("@Start");
      // //   Serial.print(batchedReferenceData[i][0]);
      // //   Serial.print(",");
      // //   Serial.print(batchedReferenceData[i][1]);
      // //   Serial.println("");
      // // }

      // Provide the past scans to be compared to the reference data
      int referenceScanIndex = 0;
      for (int j = 0; j < scanCount; j++) {
        // Taking the absolute value because the system classifies a defect as +- some error margin from the target reference point
        int voltageDifference = abs(batchedData[j][2] - detectionManager.referenceScans[referenceScanIndex].voltage);

        Serial.print("@Start");
        Serial.print(batchedData[j][0]);
        Serial.print(",");
        Serial.print(batchedData[j][1]);
        Serial.print(",");
        Serial.print(voltageDifference);
        Serial.print(",");
        Serial.print(batchedData[j][3]);
        Serial.print(",");
        Serial.println("");

        referenceScanIndex += 1;
        
        if (referenceScanIndex == 6) {
          referenceScanIndex = 0;
        }
      }
      
      Serial.println("@End");

      scanCount = 0;
    }
    delay(cycleDelay);
  }
  detectionManager.classify(lastScanBatch);
}

//LCD Messages
//Informs the user that the system is starting up
void scanStartLCD() {
  lcd.clear();
  lcd.setCursor(0,0);
  lcd.print("Starting Scan...");

  delay(1000);
}

//General diagnostic information about the system's latest readings
void displayScanInformation(
  int voltage, 
int port, 
int currentReadingBatch) {
  lcd.clear();
  lcd.noBlink();

  lcd.setCursor(0,0);
  lcd.print("Batch:" + String(currentReadingBatch) + " Port:" + String(port));
  lcd.setCursor(0,1);
  lcd.print("Voltage:" + String(voltage) + " [mV]");
}

//Converts the sensor value to a voltage [mV]
float getVoltageFrom(int sensorValue) {
  return sensorValue * (inputVoltage / 1023.0) * 1000;
}

/// Responsible for scanning, caching the scans, and providing a layer of communication with the detection manager
class ScanController {

};

/// Establishes and maintains a connection with a qualified Bluetooth host, this allows for communication with an external client
class BluetoothController {


};
