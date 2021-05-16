/*********************************************************************************************  
*   This is the first portion of the source code for a BIOMETRIC ATTENDANCE DEVICE
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
*   This src is meant to be run on the primary Atmega328 driving the touchscreen.
*
*********************************************************************************************   
*       Created: 26/06/2018 08:38:34
*/


// Include the neccessary header files
#include "Adafruit_GFX.h"       // Adafruit Graphics library
#include "Adafruit_ILI9341.h"   // ILI9341 LCD TFT screen library
#include "URTouch.h"            // Library required for the touchscreen

// Define the pins for the LCD touchscreen
#define TFT_DC 9
#define TFT_CS 10
#define TFT_RST 8
#define TFT_MISO 12
#define TFT_MOSI 11
#define TFT_CLK 13
// These pins enable the touch functionality
#define t_SCK 3
#define t_CS 4
#define t_MOSI 5
#define t_MISO 6
#define t_IRQ 7

// Assign human-readable names to some common 16-bit color values:
#define	BLACK   0x0000
#define	BLUE    0x001F
#define	RED     0xF800
#define	GREEN   0x07E0
#define CYAN    0x07FF
#define MAGENTA 0xF81F
#define YELLOW  0xFFE0
#define WHITE   0xFFFF


// Initialise screen by creating an instance of the Adafruit_ILI9341 library
Adafruit_ILI9341 tft = Adafruit_ILI9341(TFT_CS, TFT_DC, TFT_MOSI, TFT_CLK, TFT_RST, TFT_MISO);
// Initialise touchscreen
URTouch ts(t_SCK, t_CS, t_MOSI, t_MISO, t_IRQ);

// -- Setup
#define SENSITIVITY 300
#define MINPRESSURE 10
#define MAXPRESSURE 1000

// Calibrate values
#define TS_MINX 125
#define TS_MINY 85
#define TS_MAXX 965
#define TS_MAXY 905

// Global variables
int x, y, currentPage, m = 0;
char result[30];
String data;
String data1; 
char row1[10] = { 'Q', 'W', 'E', 'R', 'T', 'Y', 'U', 'I', 'O', 'P' };
char row2[10] = { 'A', 'S', 'D', 'F', 'G', 'H', 'J', 'K', 'L', ' ' };
char row3[10] = { 'Z', 'X', 'C', 'V', 'B', 'N', 'M', '/', ' ', ' ', };
char numRow1[10] = { '0', '1', '2', '3', '4', '5', '6', '7', '8', '9' };
char keys[4][10] = { { '0', '1', '2', '3', '4', '5', '6', '7', '8', '9' },
  { 'Q', 'W', 'E', 'R', 'T', 'Y', 'U', 'I', 'O', 'P' },
  { 'A', 'S', 'D', 'F', 'G', 'H', 'J', 'K', 'L', ' ' },
  { 'Z', 'X', 'C', 'V', 'B', 'N', 'M', '/', ' ', ' ' }
};

// Function prototypes
void checkPos();
void choosePage();
void welcomeScreen();
void drawPage1();
void drawPage2();
String detectButtons();
void drawPage3();
void drawPage4();
void successfulPg();
void classPg(String matric);
void errorPg()
void errorRegPg()


// Main program begins here
void setup()
{
  // Initialise touchscreen
  ts.InitTouch();
  ts.setPrecision(PREC_EXTREME);

  // We set the last index of the result array to a null character. This converts it to a string
  result[29] = { '\0' };

  // Begin the serial communication
  Serial.begin(9600);

  // Initialise the touch screen and set the rotation to landscape
  tft.begin();
  tft.setRotation(3);

  // Display the welcome screen by calling the welcomeScreen function
  welcomeScreen();
  // Wait for two seconds
  delay(2000);

  // Display page one and recieve input on which page to go next
  drawPage1();
  choosePage();
}


void loop()
{
  // The loop for this project switches on the values of the currentPage variable
  switch (currentPage)
  {
    case 1:
      currentPage = 1;
      choosePage();
      break;
    case 2:
      currentPage = 3;
      drawPage2();
      break;
    case 3:
	  // Calls the detectButtons function and returns the data
      data = detectButtons();
      data.trim();
	  // Data returned must be exactly 12 characters i.e. the standard length of a matric number
      if (data.length() == 12)
      {
		  // Send the data (matric number) to the secondary microcontroller
        Serial.print(data);
      }

      x = 0;
      y = 0;
      checkPos();

      // If the NEXT button is pressed
      if (x >= 235 && x <= 320 && y >= 210 && y <= 240)
      {
        Serial.println("$");
		  // memset clears the result array whenever the next or back button is pressed i.e. clear the matric field
        memset(result, ' ', 19);
        m = 0;
        drawPage3();
        currentPage = 4;
      }

      // If the BACK button is pressed
      if (x >= 0 && x <= 85 && y >= 210 && y <= 240)
      {
        memset(result, ' ', 19);
        m = 0;
        drawPage1();
        currentPage = 1;
      }
      break;
    case 4:
      x = 0;
      y = 0;
      checkPos();
      currentPage = 4;
      // If BACK button is pressed
      /* if (x >= 0 && x <= 85 && y >= 210 && y <= 240)
        {
         currentPage = 2;
        }*/

      Serial.setTimeout(400000);
      while (!Serial.available());
	    // Search for '~' in the serial buffer, return 1 if found, 0 otherwise
	    // This character is received from the secondary microcontroller
      data1 = Serial.find('~');
	
      if (!(data1 == 0))
      {
        currentPage = 6;
        Serial.println("Again..."); // For debugging 
        Serial.println(data1);
        successfulPg();
      }else {
        currentPage = 4;
        errorPg();
      }

      // If OK button is pressed
      if (x >= 235 && x <= 320 && y >= 210 && y <= 240)
      {
        currentPage = 1;
        drawPage1();
      }
      break;
    case 5:
      x = 0;
      y = 0;
      checkPos();
      currentPage = 5;

      // If BACK button is pressed
      if (x >= 0 && x <= 85 && y >= 210 && y <= 240)
      {
        currentPage = 1;
        drawPage1();
      }

      if (Serial.available() > 0)
      {
        String matric1 = Serial.readStringUntil('&');
        matric1.trim();
        Serial.println(matric1);
        if (matric1.length() == 12)
        {
          classPg(matric1);
        }
        else
        {
          errorRegPg();
        }
      }

      // If DONE button is pressed
      if (x >= 235 && x <= 320 && y >= 210 && y <= 240)
      {
        currentPage = 1;
        drawPage1();
      }
      break;
    case 6:
      x = 0;
      y = 0;
      checkPos();
      currentPage = 6;
      // If DONE button is pressed
      if (x >= 235 && x <= 320 && y >= 210 && y <= 240)
      {
        currentPage = 1;
        drawPage1();
      }
      break;

    default:
      break;
  }
  delay(25);

}

// This function is required to get the coordinates from a touch on the screen
void checkPos()
{
  if (ts.dataAvailable())
  {
    ts.read();
    x = ts.getX();
    y = ts.getY();
  }
}

// This function allows users to select the page they want to go to after the first welcome screen
void choosePage()
{
  x = 0;
  y = 0;

  while (true)
  {
    ts.read();      // Reads data from the touch screen
    x = ts.getX();  // gets the x-coordinate from the touch
    y = ts.getY();  // gets the y-coordinate

    // REGISTER
    if (x >= 100 && x <= 250 && y >= 50 && y <= 90) {
      currentPage = 2;
      // The "#" serves as a trigger for the secondary microcontroller to begin the registration process
      Serial.print("#");

    }
    // ATTENDANCE
    else if (x >= 80 && x <= 265 && y >= 140 && y <= 180) {
      currentPage = 5;
      // The "@" serves as a trigger for the secondary microcontroller to begin the attendance process
      Serial.print("@");
      drawPage4();
    }

    // Once the current page is greater than or equal to 1, break and exit this function
    if (currentPage >= 1) {
      break;
    }
  }
}

// Displays the welcome screen
void welcomeScreen()
{
  tft.fillScreen(WHITE);
  tft.setCursor(55, 40);
  tft.setTextSize(4);
  tft.setTextColor(BLUE);
  tft.println("BIOMETRIC");
  tft.setCursor(40, 80);
  tft.println("ATTENDANCE");
  tft.setCursor(85, 120);
  tft.println("DEVICE");
  tft.setCursor(10, 200);
  tft.setTextSize(2);
  tft.println("Developed by ");
  tft.setCursor(10, 220);
  tft.println("EEE401 Group 2");

}

// Page 1: select options screen
void drawPage1()
{
  tft.fillScreen(WHITE);
  tft.setCursor(5, 10);
  tft.setTextSize(2);
  tft.setTextColor(BLUE);
  tft.println("Please, select your choice:");
  tft.fillRoundRect(100, 50, 150, 40, 10, MAGENTA);
  tft.fillRoundRect(80, 140, 185, 40, 10, MAGENTA);
  tft.setCursor(105, 60);
  tft.setTextSize(3);
  tft.setTextColor(BLACK);
  tft.println("REGISTER");
  tft.setCursor(85, 150);
  tft.setTextSize(3);
  tft.setTextColor(BLACK);
  tft.println("ATTENDANCE");
}

// Page 2: Register page
void drawPage2()
{
  tft.fillScreen(BLACK);
  // Draw the display window
  tft.fillRect(0, 0, 320, 40, WHITE);
  // Draw Horizontal Lines
  for (int h = 40; h <= 200; h += 40)
    tft.drawFastHLine(0, h, 320, WHITE);
  // Draw Vertical Lines
  for (int v = 30; v <= 320; v += 32)
    tft.drawFastVLine(v, 40, 160, WHITE);
  tft.fillRect(256, 161, 62, 39, BLACK);

  // Display Keypad labels
  for (int i = 0; i < 10; i++)
  {
    tft.setCursor(10 + (32 * i), 55);
    tft.setTextSize(2);
    tft.setTextColor(WHITE);
    tft.println(numRow1[i]);
  }
  for (int i = 0; i < 10; i++)
  {
    tft.setCursor(10 + (32 * i), 90);
    tft.setTextSize(2);
    tft.setTextColor(WHITE);
    tft.println(row1[i]);
  }
  for (int i = 0; i < 10; i++)
  {
    tft.setCursor(10 + (32 * i), 130);
    tft.setTextSize(2);
    tft.setTextColor(WHITE);
    tft.println(row2[i]);
  }
  for (int i = 0; i < 10; i++)
  {
    tft.setCursor(10 + (32 * i), 170);
    tft.setTextSize(2);
    tft.setTextColor(WHITE);
    tft.println(row3[i]);
  }
  tft.setCursor(262, 174);
  tft.println("<--");

  tft.fillRoundRect(235, 210, 85, 30, 10, RED);
  tft.setTextSize(2);
  tft.setTextColor(BLACK);
  tft.setCursor(250, 220);
  tft.println("NEXT>");
  tft.fillRoundRect(0, 210, 85, 30, 10, RED);
  tft.setTextSize(2);
  tft.setTextColor(BLACK);
  tft.setCursor(3, 220);
  tft.println("<BACK");
  tft.setCursor(5, 15);
  tft.print("Enter your MATRIC NO.");
  delay(1500);
  tft.fillRect(0, 0, 320, 40, WHITE);

  detectButtons();
}

// Read buttons pressed on the touchscreen by the user
String  detectButtons()
{
  String matric;
  int x, y, k;
  if (ts.dataAvailable())
  {
    ts.read();
    x = ts.getX();
    y = ts.getY();

    for (int i = 1; i <= 10; i++)
    {
      for (int j = 1; j <= 4; j++)
      {
        if (x >= ((i - 1) * 32) && x <= (((i - 1) * 32) + 32) && y <= (j * 40) + 40 && y >= j * 40 && m < 26)
        {
          //Backspace is pressed
          if (x >= 256 && x <= 319 && y >= 161 && y <= 200 && m > 0)
          {
            result[m - 1] = ' ';
            tft.setCursor(5, 15);
            m--;
          }

          else {
            char display = keys[j - 1][i - 1];
            tft.setCursor(5, 15);
            result[m] = display;
            m++;
          }
          tft.fillRect(0, 0, 320, 40, WHITE);
          tft.setCursor(5, 15);
          tft.print(result);
          matric = String(result);
        }
      }
    }
  }
  return matric;
}

// Fingerprint page
void drawPage3()
{
  tft.fillScreen(BLACK);
  tft.setTextColor(BLUE);
  tft.setTextSize(2);
  tft.setCursor(30, 10);
  tft.print("BIOMETRIC REGISTRATION");
  tft.setTextColor(WHITE);
  tft.setCursor(10, 40);
  tft.print("Please place your finger");
  tft.setCursor(10, 60);
  tft.print("on the scanner to provide");
  tft.setCursor(10, 80);
  tft.print("biometric information!");
  tft.setCursor(10, 130);
  tft.print("When the scanner lights");
  tft.setCursor(10, 150);
  tft.print("up, place your finger.");
  tft.setCursor(10, 170);
  tft.print("Place the same finger ");
  tft.setCursor(10, 190);
  tft.print("three times.");
  /*tft.drawFastHLine(100, 50, 100, WHITE);
    tft.drawFastHLine(100, 125, 100, BLUE);
    tft.drawFastHLine(100, 200, 100, WHITE);
    tft.drawFastVLine(100, 50, 150, WHITE);
    tft.drawFastVLine(200, 50, 150, WHITE);
    tft.fillRoundRect(235, 210, 85, 30, 10, RED);
    tft.setTextSize(2);
    tft.setTextColor(BLACK);
    tft.setCursor(250, 220);
    tft.println("DONE");
    tft.fillRoundRect(0, 210, 85, 30, 10, RED);
    tft.setTextSize(2);
    tft.setTextColor(BLACK);
    tft.setCursor(3, 220);
    tft.println("<BACK");*/
}

// Attendance page
void drawPage4()
{
  tft.fillScreen(WHITE);
  tft.setTextColor(BLACK);
  tft.setTextSize(2);
  tft.setCursor(10, 20);
  tft.print("Place your finger on the ");
  tft.setCursor(80, 80);
  tft.setTextSize(4);
  tft.setTextColor(BLUE);
  tft.print("SENSOR.");
  /*tft.fillRoundRect(235, 210, 85, 30, 10, RED);
    tft.setTextSize(2);
    tft.setTextColor(BLACK);
    tft.setCursor(250, 220);
    tft.println("DONE");*/
  tft.fillRoundRect(0, 210, 85, 30, 10, RED);
  tft.setTextSize(2);
  tft.setTextColor(BLACK);
  tft.setCursor(3, 220);
  tft.println("<BACK");
}

// Draw successful page
void successfulPg()
{
  tft.fillScreen(WHITE);
  tft.setTextSize(4);
  tft.setTextColor(GREEN);
  tft.setCursor(20, 70);
  tft.print("SUCCESSFULLY");
  tft.setCursor(30, 110);
  tft.print("REGISTERED!");
  tft.fillRoundRect(235, 210, 85, 30, 10, RED);
  tft.setTextSize(2);
  tft.setTextColor(BLACK);
  tft.setCursor(250, 220);
  tft.println("DONE");
}

// Successful attendance
void classPg(String matric)
{
  tft.fillScreen(WHITE);
  tft.setTextSize(3);
  tft.setTextColor(BLUE);
  tft.setCursor(110, 30);
  tft.print("Hello!");
  tft.setTextSize(4);
  tft.setCursor(20, 80);
  tft.setTextColor(GREEN);
  tft.print(matric);
  tft.setTextSize(2);
  tft.setCursor(10, 130);
  tft.setTextColor(BLUE);
  tft.print("Welcome to today's class!");
  tft.fillRoundRect(235, 210, 85, 30, 10, RED);
  tft.setTextSize(2);
  tft.setTextColor(BLACK);
  tft.setCursor(250, 220);
  tft.println("DONE");
}

// Error on attendance page
void errorPg()
{
  tft.fillScreen(BLACK);
  tft.setTextSize(4);
  tft.setTextColor(RED);
  tft.setCursor(90, 20);
  tft.print("ERROR!");
  tft.setTextSize(2);
  tft.setCursor(10, 100);
  tft.print("Inconsistent Fingerprint.");
  tft.setTextColor(WHITE);
  tft.setCursor(10, 220);
  tft.print("Please try again!");
  tft.fillRoundRect(235, 210, 85, 30, 10, RED);
  tft.setTextSize(2);
  tft.setTextColor(BLACK);
  tft.setCursor(270, 220);
  tft.println("OK");
}

// Error on Registration page
void errorRegPg()
{
  tft.fillScreen(BLACK);
  tft.setTextSize(4);
  tft.setTextColor(RED);
  tft.setCursor(90, 20);
  tft.print("ERROR!");
  tft.setTextSize(2);
  tft.setCursor(10, 100);
  tft.print("It seems you are not");
  tft.setCursor(10, 120);
  tft.print("registered for this");
  tft.setCursor(10, 140);
  tft.print("course.");
  tft.setCursor(10, 180);
  tft.setTextColor(WHITE);
  tft.print("Please, REGISTER first!");
  tft.fillRoundRect(235, 210, 85, 30, 10, RED);
  tft.setTextSize(2);
  tft.setTextColor(BLACK);
  tft.setCursor(270, 220);
  tft.println("OK");
}
