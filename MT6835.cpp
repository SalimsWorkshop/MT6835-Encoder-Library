#include "MT6835.h"

// statische Zeiger initialisieren
MT6835* MT6835::instance1 = nullptr;
MT6835* MT6835::instance2 = nullptr;

// SPI SELECT
SPIClass* selectSPI(uint8_t CS_PIN) {
    if (CS_PIN == 0) return &SPI1;
    if (CS_PIN == 10) return &SPI;
    if (CS_PIN == 36) return &SPI2;
    return &SPI; // Fallback
} // ===========================================


// SENSOR OBJECT
MT6835::MT6835(uint8_t CS_PIN, uint32_t SPI_FREQ, uint8_t A_PIN, uint8_t B_PIN) 
    : // Initializing the Parameters
    CS_PIN(CS_PIN), SPI_FREQ(SPI_FREQ), SPI_SETTINGS(SPI_FREQ, MSBFIRST, SPI_MODE3), //Setting the SPI Settings
    A_PIN(A_PIN), B_PIN(B_PIN),
    LAST_TICKS(0), INCR_ANGLE(0.0f), 
    position(0), lastCycle(0), lastDt(0), lastDir(1), lastA(0), lastB(0),
    ANG_VEL_RAW(0.0f), ANG_VEL_FILT(0.0f) { 
        SPI = selectSPI(CS_PIN);            // Choosing the right SPI Interface
        // instance = this;
        if (instance1 == nullptr) instance1 = this;
        else if (instance2 == nullptr) instance2 = this;


} // ===========================================


// INITIALIZE 
void MT6835::initialize() {

    pinMode(CS_PIN, OUTPUT);                 // So we can toggle it
    digitalWrite(CS_PIN, HIGH);              // CS high
    SPI->begin();                            // start our SPI
    setABZResolution(true,false,ABZ_RES_VAL); // Set Resolution after startup
    getABZResolution();                      // Print Resolution after startup

    // Encoder INIT
    pinMode(A_PIN, INPUT_PULLUP);
    pinMode(B_PIN, INPUT_PULLUP);

    lastA = digitalRead(A_PIN);
    lastB = digitalRead(B_PIN);
    lastCycle = getCycleCount();

    // attach Interrupts
    if(this == instance1) {
        attachInterrupt(digitalPinToInterrupt(A_PIN), encoderISRWrapper1, CHANGE);
        attachInterrupt(digitalPinToInterrupt(B_PIN), encoderISRWrapper1, CHANGE);
    } else if(this == instance2) {
        attachInterrupt(digitalPinToInterrupt(A_PIN), encoderISRWrapper2, CHANGE);
        attachInterrupt(digitalPinToInterrupt(B_PIN), encoderISRWrapper2, CHANGE);
    }

    delay(100);                              // just for fun
} // ===========================================


// Static ISR Wrappers
void MT6835::encoderISRWrapper1() {
    if(instance1) instance1->encoderISR();
}
void MT6835::encoderISRWrapper2() {
    if(instance2) instance2->encoderISR();
}


// Minimal-ISR für Incremental Encoder
void MT6835::encoderISR() {
    uint32_t c = getCycleCount();
    lastDt = c - lastCycle;
    lastCycle = c;

    int a = digitalRead(A_PIN);
    int b = digitalRead(B_PIN);

    if (a != lastA) lastDir = (a == b) ? 1 : -1;
    else if (b != lastB) lastDir = (a != b) ? 1 : -1;

    position += lastDir;
    lastA = a;
    lastB = b;
}




// Winkel über Incremental Encoder
float MT6835::readIncAng() {
    // long pos;
    noInterrupts();
    pos = position;
    interrupts();

    INCR_ANGLE = pos * TICK_ANGLE; // oder TICKS_PER_REV falls anders
    return INCR_ANGLE;
}


int32_t MT6835::readRawIncAng(){
    // long pos;
    noInterrupts();
    pos = position;
    interrupts();

    return pos;

}


float MT6835::readAngVelRaw(){
    // static long oldPos = 0;
    // static uint32_t oldTime = 0;

    // noInterrupts();
    //   dt = lastDt;
    //   dir = lastDir;
    //   lastPulse = lastCycle;
    //   pos = position;
    // interrupts();

    // lokale Kopie, Interrupts kurz deaktivieren
    long posLocal;
    int dirLocal;
    uint32_t dtLocal, lastPulseLocal;

    noInterrupts();
      posLocal = position;
      dirLocal = lastDir;
      dtLocal  = lastDt;
      lastPulseLocal = lastCycle;
    interrupts();

    uint32_t now = getCycleCount();

    if ((now - lastPulseLocal) > 300000) { // keine Bewegung
        ANG_VEL_RAW = 0.0f;
    } 
    else if (dtLocal > 0 && dtLocal < 300000) { // schnelle Drehung
        float seconds = dtLocal / 600'000'000;
        ANG_VEL_RAW = dirLocal * (1.0f / (seconds * TICKS_PER_REV));
    } 
    else { // langsame Drehung: Fenster-Methode
        long deltaPos = posLocal - oldPosRaw;
        uint32_t deltaT = now - oldTimeRaw;
        if (deltaT > 0) {
            float seconds = deltaT / 600'000'000;
            ANG_VEL_RAW = (deltaPos / (float)TICKS_PER_REV) / seconds;
        }
        oldPosRaw = posLocal;
        oldTimeRaw = now;
    }

    return ANG_VEL_RAW * TWO_PI;

}



float MT6835::readAngVelFilt(float ALPHA_FILT) {
    float BETA_FILT = 1 - ALPHA_FILT;

    return ANG_VEL_FILT = ALPHA_FILT * readAngVelRaw() + BETA_FILT * ANG_VEL_FILT;

}


// ABSOLUTE ANGLE READING
float MT6835::readAbsAng() {         // Burst  READ

  uint8_t DATA[6] = {0};                    // transact 48 bits
          DATA[0] = (BURST_CMD << 4);       // Burst command
          DATA[1] = ANGLE_REG_MSB;          // Start Register, rest is dummy

    SPI->beginTransaction(SPI_SETTINGS);
        // digitalWrite(CS_PIN, LOW);
        digitalWriteFast(CS_PIN,LOW);
        // csnLow();
            SPI->transfer(DATA,6);          // Burst all data
        // digitalWrite(CS_PIN, HIGH);
        digitalWriteFast(CS_PIN,HIGH);
        // csnHigh();
    SPI->endTransaction();

       // Bits zusammensetzen: [20:13] [12:5] [4:0]
    int32_t raw = (DATA[2] << 13) |
                   (DATA[3] << 5)  |
                   (DATA[4] >> 3);

    raw &= 0x1FFFFF; // sicherstellen: nur 21 bit

    return raw * RAD_PER_LSB;   // Werte 0 .. (1<<21)-1
}// ===========================================


// ABSOLUTE ANGLE RAW
int32_t MT6835::readRawAbsAng(){

  uint8_t DATA[6] = {0};                    // transact 48 bits
          DATA[0] = (BURST_CMD << 4);       // Burst command
          DATA[1] = ANGLE_REG_MSB;          // Start Register, rest is dummy

    SPI->beginTransaction(SPI_SETTINGS);
        digitalWriteFast(CS_PIN, LOW);
            SPI->transfer(DATA,6);          // Burst all data
        digitalWriteFast(CS_PIN, HIGH);
    SPI->endTransaction();

       // Bits zusammensetzen: [20:13] [12:5] [4:0]
    int32_t raw = (DATA[2] << 13) |
                   (DATA[3] << 5)  |
                   (DATA[4] >> 3);

    raw &= 0x1FFFFF; // sicherstellen: nur 21 bit
    return raw;   // Werte 0 .. (1<<21)-1
}// ===========================================



// CURRENT ABZ RESOLUTION READING
uint16_t MT6835::getABZResolution(){
    SPI->beginTransaction(SPI_SETTINGS);
        digitalWrite(CS_PIN, LOW);
            SPI->transfer(READ_CMD << 4);           // Read Command
            SPI->transfer(ABZ_RES_REG_MSB);         // Register 0x007
            uint16_t MSB = SPI->transfer(0x00);     // Read and save Data of 0x007
        digitalWrite(CS_PIN, HIGH);
        digitalWrite(CS_PIN, LOW);
            SPI->transfer(READ_CMD << 4);           // Read Command 
            SPI->transfer(ABZ_RES_REG_LSB);         // Register 0x008
            uint16_t LSB = SPI->transfer(0x00);     // Read and save Data of 0x008
        digitalWrite(CS_PIN, HIGH);
     SPI->endTransaction();

    uint16_t RES =  ((MSB << 6) | (LSB >> 2)) +1 ;
    return (Serial.print("ABZ Resolution:\t"), Serial.print(RES), Serial.println(" PPR"));
} // ===========================================






// ABZ RESOLUTION SETTING
void MT6835::setABZResolution(bool ABZ_ON, bool SWAP_DIR, uint16_t ABZ_RES_VAL){ 
    SPI->beginTransaction(SPI_SETTINGS);
        digitalWrite(CS_PIN, LOW);
            SPI->transfer(WRITE_CMD << 4);              // Write Command
            SPI->transfer(ABZ_RES_REG_MSB);             // Write Register 0x007
            SPI->transfer((ABZ_RES_VAL >> 6)&0xFF);     // 16bits shifted 6bits right and take the lowest 8 bit
        digitalWrite(CS_PIN, HIGH);
        digitalWrite(CS_PIN, LOW);
            SPI->transfer(WRITE_CMD << 4);              // Write Command
            SPI->transfer(ABZ_RES_REG_LSB);             // Write Register 0x008
            // 6 lowest bits of 16bits shifted 2bits left,  if ABZ_ON true->0,   if SWAP_DIR true->1
            uint8_t LSB = ((ABZ_RES_VAL & 0x3F) << 2) | ((ABZ_ON ? 0 : 1) << 1) | (SWAP_DIR ? 1 : 0);
            SPI->transfer(LSB);
        digitalWrite(CS_PIN, HIGH);
     SPI->endTransaction();
} // ===========================================


