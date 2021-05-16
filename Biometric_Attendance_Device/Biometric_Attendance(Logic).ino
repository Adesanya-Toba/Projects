/******************************************************************************************************  
*   This is the second portion of the source code for a BIOMETRIC ATTENDANCE DEVICE
*   developed by the group 2 members of the EEE-401 Project Design course,
*   at the department of Electronic & Electrical Engineering, Obafemi Awolowo University.
*   
*   The project utilizes a 2.8" 320x240 TFT LCD Touchscreen and a Fingerprint scanner.
*   Two Atmega328 microcontrollers were used for this project which communicated with one another over
*   UART (Serial communication).
*   A "throw and catch" protocol was implemented to ensure the correct data was
*   being sent and received. This involved sending 'unique symbols' alongside the data 
*   for authentication purposes.
*   
*   This src is meant to be run on the secondary Atmega328 providing the logic and controls to 
*   the Fingerprint scanner and SD-Card.
*  
******************************************************************************************************* 
*       Created: 17/09/2018 18:31:52
*/


// Include the neccessary header files
#include <SPI.h>
#include <SD.h>
#include <EEPROM.h>
#include "FPS_GT511C3.h"
#include "SoftwareSerial.h"

// Set up software serial pins for Fingerprint scanner
// FPS (TX) is connected to pin 4 (Arduino's Software RX)
// FPS (RX) is connected through a converter to pin 5 (Arduino's Software TX)
FPS_GT511C3 fps(9, 8); // (Arduino SS_RX = pin 4, Arduino SS_TX = pin 5)

/*
*   SD card attached to SPI bus as follows:
*     ** MOSI - pin 11
*     ** MISO - pin 12
*     ** CLK - pin 13
*     ** CS - pin 4 (for MKRZero SD: SDCARD_SS_PIN)
*/

// Function prototypes
void Register();
void Attendance();
int Enroll();


// Main program begins here
void setup()
{
  Serial.begin(9600);
  // Serial.println("Initializing SD-Card");

  fps.Open();           // send serial command to initialize fps
  fps.SetLED(false);    // turn on LED so fps can see fingerprint

  pinMode(10, OUTPUT);

  // Check if the card is ready
  if (!SD.begin(10))
  {
    // Serial.println("Card Failed");
    return;
  }
  Serial.println("CARD READY");
  String identify = "ID";
  File datafile = SD.open("Log2.csv", FILE_WRITE);
  datafile.close();


  File dataCheck = SD.open("Log2.csv");
  if (dataCheck) {
    String token = dataCheck.readStringUntil(',');
    dataCheck.close();
    if (!(token == identify )) // A new FILE
    {
      File dataCheck1 = SD.open("Log2.csv", FILE_WRITE);
      if (dataCheck1) {
        String header = "ID, FINGER ID, MATRIC NO.";
        dataCheck1.println(header);
        dataCheck1.flush();
        dataCheck1.close();
        // Serial.println("We just created a new file!");
        int idNum = 1;
        int eeAddress = 0;
        EEPROM.put(eeAddress, idNum);
        fps.DeleteAll();
      }
    }

  }
  else
  {
    //Serial.println("Could not access file");
  }

}

void loop()
{
  // Loop structure is fairly simple. 
  // Wait for data from primary MCU and execute functions based on data token.
  if (Serial.available() > 0) {
    char trig = Serial.read();
    
    if (trig == '#') {
      //Serial.flush();
      Register();
    }
    else if (trig  == '@') {
      Attendance();
    }
  }
}

// Registers the student 
void Register()
{
  int idNum;
  int temp = 0;
  int eeAddress = 0;
  EEPROM.get(eeAddress, temp);
  idNum = temp;
  int fingerId = 0;

  //Serial.println("Enter your MATRIC No.: ");
  while (!Serial.available());
  String matric = Serial.readStringUntil('$');
  matric.trim();
  //Serial.println(matric);
  //Serial.print("$");
  fingerId = Enroll();
  if (!(fingerId == 900))
  {
    String dataString = String(idNum) + ", " + "0x" + String(fingerId) + ", " + String(matric);
    // Open a file to write to
    // Only one file can be open at a time
    File datafile = SD.open("Log2.csv", FILE_WRITE);
    if (datafile)
    {
      datafile.println(dataString);
      datafile.close();
      //Serial.println(dataString);
      Serial.print("~");
    }
    else
    {
      //Serial.println("Could not open Log2.csv");
      return;
    }
    idNum++;
    EEPROM.put(eeAddress, idNum);
  }
  else {
    /*Do nothing*/
  }
}

// Update student's attendance information
void Attendance()
{
  fps.SetLED(true);   // turn on LED so fps can see fingerprint
  // Identify fingerprint test
  while (true) {
    //Serial.println("Please press finger");
    if (fps.IsPressFinger())
    {
      File dataFile = SD.open("Log2.csv");
      //Serial.println("Please press finger");
      fps.CaptureFinger(false);
      int id = fps.Identify1_N();
      if (dataFile) {
        if (id >= 10 && id != 200)
        {
          String token1 =  "0x" + String(id);
          token1.trim();
          const char* v = token1.c_str();
          delay(1000);
          //const char* me = "0x3";
          //Serial.println(v);
          fps.SetLED(false);
          delay(1000);
          String token2 = dataFile.readString();
          const char* p;
          p = token2.c_str();
          String token = strstr(p, v);
          token.trim();
          //Serial.println(token);
          String kay = token.substring(6, 18);
          Serial.print(kay);
          Serial.print("&");
          //Serial.println(kay.length());
          //Serial.println("We opened Log2.csv");
        }
        else if ( id < 10 && id != 200) {
          String token1 =  "0x" + String(id);
          token1.trim();
          const char* v = token1.c_str();
          //delay(3000);
          //const char* me = "0x3";
          //Serial.println(v);
          fps.SetLED(false);
          //delay(5000);
          String token2 = dataFile.readString();
           //delay(5000);
          const char* p;
          p = token2.c_str();
          //Serial.println(p);
          String token = strstr(p, v);
          token.trim();
         // Serial.println(token);
          String kay = token.substring(5, 17);
          Serial.println(kay);
          Serial.print("&");
          // Serial.println(kay.length());
          // Serial.println("We opened Log2.csv");
        }
        else {
          Serial.println("Not registered&");
          fps.SetLED(false);
        }
      }
      else {
        // Serial.println("Could not open Log2.csv");
      }
      delay(1000);
      dataFile.close();
      break;
    }

    else
    {
      // Serial.println("Please press finger");
    }
  }
}

// Stores student's fingerprint information in FPS EEPROM
int Enroll()
{
  // fps.DeleteAll();
  // find open enroll id
  int enrollid = 0;
  bool usedid = true;
  while (usedid == true)
  {
    usedid = fps.CheckEnrolled(enrollid);
    if (usedid == true) enrollid++;
  }
  fps.EnrollStart(enrollid);
  fps.SetLED(true);
  
  // enroll
  //Serial.print("Press finger to Enroll #");
  //  Serial.println(enrollid);
  while (fps.IsPressFinger() == false) delay(100);
  bool bret = fps.CaptureFinger(true);
  int iret = 0;
  if (bret != false)
  {
    // Serial.println("Remove finger");
    fps.SetLED(false);
    fps.Enroll1();
    while (fps.IsPressFinger() == true) delay(1000);
    fps.SetLED(true);
    //Serial.println("Press same finger again");
    while (fps.IsPressFinger() == false) delay(1000);
    bret = fps.CaptureFinger(true);
    if (bret != false)
    {
      // Serial.println("Remove finger");
      fps.SetLED(false);
      fps.Enroll2();
      while (fps.IsPressFinger() == true) delay(1000);
      fps.SetLED(true);
      //Serial.println("Press same finger yet again");

      while (fps.IsPressFinger() == false) delay(1000);
      bret = fps.CaptureFinger(true);
      if (bret != false)
      {
        // Serial.println("Remove finger");
        fps.SetLED(false);
        iret = fps.Enroll3();
        if (iret == 0)
        {
          // Serial.println("Enrolling Successful!");
          //Serial.print("Your id is ");
          //Serial.println(enrollid);
          return enrollid;
        }
        else
        {
          //Serial.print("Enrolling Failed with error code:");
          //Serial.println(iret);
          //Serial.print("?");
          enrollid = 900;
          return enrollid;
        }
      }
      else {
        enrollid = 900;
        return enrollid;
      } //Serial.println("Failed to capture third finger");
    }
    else {
      enrollid = 900;
      return enrollid;
    } //Serial.println("Failed to capture second finger");
  }
  else {
    enrollid = 900;
    return enrollid;
  } //Serial.println("Failed to capture first finger");
}
