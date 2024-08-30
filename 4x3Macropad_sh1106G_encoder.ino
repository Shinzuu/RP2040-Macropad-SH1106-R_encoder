#include <Adafruit_SH110X.h>
#include <Wire.h>
#include <Keyboard.h>

// OLED display setup for SH1106G
Adafruit_SH1106G display = Adafruit_SH1106G(128, 64, &Wire, -1);

// Button pins
const int buttonPins[] = {4, 9, 14, 21,
 5, 8, 13, 16,
 6, 7, 12, 15};
const int numButtons = 12;
bool buttonStates[numButtons];
bool lastButtonStates[numButtons];

// Encoder pins
const int encoderPinA = 20;
const int encoderPinB = 23;
const int encoderButtonPin = 22;

// Encoder state
int lastEncoderA = LOW;
int encoderPos = 0;
bool lastEncoderButtonState = false;

const int numLayers = 5;
const char* layerNames[numLayers] = {"test", "Photoshop", "Premier", "DocX", "Fusion360"};
const char* layerFunctions[numLayers][12] = {
  {"Q", "W", "E", "R", "A", "S", "D", "F", "Z", "X", "C", "V"},
  {"B", "E", "Z", "R", "C", "L", "T", "U", "R", "F", "S", "S"},
  {"P", "A", "S", "N", "P", "V", "M", "S", "R", "S", "C", "C"},
  {"N", "O", "S", "P", "U", "R", "B", "I", "U", "F", "R", "A"},
  {"S", "M", "R", "S", "M", "E", "R", "C", "F", "C", "P", "U"}
};

int currentLayer = 0;

void setup() {
  Serial.begin(115200);
  delay(100);  // RP2040 delay is not a bad idea

  // Start OLED
  Wire.setSDA(0);
  Wire.setSCL(1);
  Wire.begin();
  display.begin(0x3C, true); // I2C address
  display.display();
  
  // Display startup animation
  for (int i = 0; i < 3; i++) {
    display.clearDisplay();
    display.setTextSize(2);
    display.setCursor(0, 10);
    display.setTextColor(SH110X_WHITE);
    display.println("Welcome!");
    display.display();
    delay(100);
    
    display.clearDisplay();
    display.setCursor(0, 10);
    display.setTextSize(2);
    display.setTextColor(SH110X_WHITE);
    display.println("Loading...");
    display.display();
    delay(100);
  }

  // Initialize button pins
  for (int i = 0; i < numButtons; i++) {
    pinMode(buttonPins[i], INPUT_PULLUP); // Set each button pin as input with internal pull-up resistor
    lastButtonStates[i] = false; // Initialize last button states
  }

  // Initialize encoder pins
  pinMode(encoderPinA, INPUT_PULLUP);
  pinMode(encoderPinB, INPUT_PULLUP);
  pinMode(encoderButtonPin, INPUT_PULLUP);
}

void loop() {
  // Encoder handling
  int encoderA = digitalRead(encoderPinA);
  int encoderB = digitalRead(encoderPinB);
  bool encoderButtonState = digitalRead(encoderButtonPin) == LOW;

  // Update encoder position
  if (encoderA != lastEncoderA) {
    if (encoderA == LOW && encoderB == HIGH) {
      encoderPos++;
    } else if (encoderA == HIGH && encoderB == LOW) {
      encoderPos--;
    }
    lastEncoderA = encoderA;
    currentLayer = (encoderPos + numLayers) % numLayers;
    displayLayer();
  }

  // Update encoder button
  if (encoderButtonState != lastEncoderButtonState) {
    if (encoderButtonState) {
      // Handle encoder button press
      Serial.print("Layer ");
      Serial.println(currentLayer + 1);
      // Send HID report if needed
    }
    lastEncoderButtonState = encoderButtonState;
  }

  // Button handling
  for (int i = 0; i < numButtons; i++) {
    buttonStates[i] = !digitalRead(buttonPins[i]); // Read each button state (active low)
    
    if (buttonStates[i] != lastButtonStates[i]) {
      if (buttonStates[i]) {
        // Send HID report
        // Map layerFunctions to ASCII key values (adjust as needed)
        String funcName = layerFunctions[currentLayer][i];
        if (funcName.length() > 0) {
          char key = funcName[0]; // Get the first character of the function name
          Keyboard.press(key);
          delay(50);
          Keyboard.release(key);
          Serial.print("Button ");
          Serial.print(i + 1);
          Serial.print(" Pressed: ");
          Serial.println(key);
        }
      } else {
        Serial.print("Button ");
        Serial.print(i + 1);
        Serial.println(" Released");
      }
      lastButtonStates[i] = buttonStates[i]; // Update last state
    }
  }

  display.display(); // Show the button states on the OLED
  delay(100); // Short delay between readings
}

void displayLayer() {
  display.clearDisplay();
  display.setTextSize(1);
  display.setTextColor(SH110X_WHITE);

  // Display layer name on the left side
  display.setCursor(0, 0); // Adjust cursor to the left side
  display.setTextSize(1);
  display.print(layerNames[currentLayer]);

  // Display button functions in a 3x4 grid
  int xOffset = 40; // Leave space for layer name
  int yOffset = 10; // Starting vertical position
  int cellWidth = 32; // Width of each cell
  int cellHeight = 16; // Height of each cell

  for (int row = 0; row < 3; row++) {
    for (int col = 0; col < 4; col++) {
      display.setCursor(xOffset + col * cellWidth, yOffset + row * cellHeight); // Calculate position for each cell
      String funcName = layerFunctions[currentLayer][row * 4 + col];
      funcName += "   "; // Add extra space to fill the section
      display.print(funcName.substring(0, 3)); // Display only the first 3 characters (abbreviation)
    }
  }

  display.display();
}
