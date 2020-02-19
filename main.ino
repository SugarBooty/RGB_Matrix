/* 
 * Pinout:
 *  d2 - 4017 reset
 *  d3 - 4017 clock
 *  
 *  d5 - shift clock
 *  d6 - shift clear
 *  d7 - shift ser in
 * 
 *  d9 - blank
 */

/**
 * 24 bits, 0-255, 8 LEDs per row
 * 010 110 101 011 101 010 110 010
 *
 * OOO: Send 24 bits, row by row, for 8 Columns
 *  Example input when using Serial Monitor: 
 *    010 110 101 011 101 010 110 010 (enter)
 *    010 110 101 011 101 010 110 010 (enter)
 *    010 110 101 011 101 010 110 010 (enter)
 *    010 110 101 011 101 010 110 010 (enter)
 *    010 110 101 011 101 010 110 010 (enter)
 *    010 110 101 011 101 010 110 010 (enter)
 *    010 110 101 011 101 010 110 010 (enter)
 *    010 110 101 011 101 010 110 010
 *   *rolls over*
*/

// Consants
/* In Physical Pin Order */
const int DISPL_RST = D7;
// Ground Pin
const int a4017_CLK = D6;
const int a4017_RST = D5;
const int SHIFT_CLK = D4;
const int SHIFT_CLR = D3;
const int SHIFT_SER = D1;
// Not Connected
// Vcc
/* ----- */
const int updateWait = 8;

// Buffers ---- [col][bytes per row]
// 32 bytes per frame
byte frameBuffer[8][4];
byte loadBuffer[8][4];

// Keep track of current col and bytes TotalShifted
// to know when a new frame is available
byte rowBytesShifted = 0;
byte currentCol = 0;

// Timekeeping
unsigned long curTime = 0;
unsigned long lastTimeGen = 0;
unsigned long lastTimeUpdate = 0;

void setup() {
    pinMode(SHIFT_SER, OUTPUT);
    pinMode(SHIFT_CLR, OUTPUT);
    pinMode(SHIFT_CLK, OUTPUT);
    pinMode(a4017_RST, OUTPUT);
    pinMode(a4017_CLK, OUTPUT);
    pinMode(DISPL_RST, OUTPUT);

    // Resets the 4017
    digitalWrite(a4017_RST, HIGH);
    delay(1);
    digitalWrite(a4017_RST, LOW);

    // Initiates serial communication
    Serial.begin(115200);
    Serial.write("READY");
    Serial.flush();

    // Set the shift register reset pin
    digitalWrite(SHIFT_CLR, HIGH);
}

void clearReg() {
    // Sends a pulse to the clear pin on the shift registers
    digitalWrite(SHIFT_CLR, LOW);
    delayMicroseconds(1);
    digitalWrite(SHIFT_CLR, HIGH);
}

void nextColumb() {
    // increments 4017 clock pin
    digitalWrite(a4017_CLK, HIGH);
    delayMicroseconds(5);
    digitalWrite(a4017_CLK, LOW);
}

void showDisplay(unsigned short delay = 20) {
    digitalWrite(DISPL_RST, HIGH);
    delayMicroseconds(delay);
    digitalWrite(DISPL_RST, LOW);
}


void updateFrameBuffer() {
    Serial.println("New frame! Updating frame buffer");
    for (int col = 0; col < 8; col++) {
        for (int row = 0; row < 4; row++) {
            frameBuffer[col][row] = loadBuffer[col][row];
        }
    }
}
void updateFrame() {
    for (int col = 0; col < 8; col++) {
        for (int row = 0; row < 3; row++) {
            shiftOut(SHIFT_SER, SHIFT_CLK, LSBFIRST, ~frameBuffer[col][row]);
        }
        clearReg();
        nextColumb();
    }
}


// 1 byte per 2 LEDs
// 10110110 01101101 11010101 00110011
// RGBRGB-- RGBRGB-- RGBRGB-- RGBRGB--

// Keep track of current col and bytes TotalShifted
// to know when a new frame is available
// byte rowBytesShifted = 0;
// byte currentCol = 0;

void readFromSerial() {
    // this function only works with 3 bytes at a time
    // as 3 is how many bytes per row we have
    for (int i = 0; i < 3; ++i) {
        byte currentByte = Serial.read();
        Serial.print("incoming byte: ");
        Serial.println(currentByte);
        Serial.print("TotalShifted: ");
        Serial.println(totalBytesShifted);
        loadBuffer[currentCol][i] = currentByte;
        rowBytesShifted += 1;
        if (rowBytesShifted == 3) {
            rowBytesShifted = 0;
            currentCol += 1;
        }
        if (currentCol == 8) {
            currentCol = 0;
            updateFrameBuffer();
        }
    }
}

// Loop:
// Check if time > amount needed
// - if new frame, update buffer, blink frame
// - if not, blink old frame buffer
// --
// Check if data in Serial buffer
// - if so, read it into local buffer
// - if buffer fully updated, set new frame = true
//

void loop() {
    curTime = millis();

    if ((curTime - lastTimeUpdate) >= updateWait) {
        lastTimeUpdate = curTime;
        showDisplay();
    }

    if (Serial.available() >= 3) {
        Serial.print("Amount of bytes in buffer: ");
        Serial.println(Serial.available());
        readFromSerial();
    }
}