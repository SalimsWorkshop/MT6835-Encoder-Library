
#include "MT6835.h"

// Magnetic Encoder Class
MT6835 mt6835(CS_PIN_MOT, SPI_FREQ_MOT, ABZ_A_PIN_MOT, ABZ_B_PIN_MOT); // CS_PIN, SPI_FREQ, A_PIN, B_PIN

void setup() {
    Serial.begin(912600);
    while(!Serial) {} // Wait for Serial to initialize

    // Encoder initialization
        mt6835.initialize();

    delay(1000);
}


void loop() { 

    // Absolute Angle
    float abs = mt6835.readAbsAng(); 
    // Serial.print(">abs: ");   Serial.println(abs, 5);  

    // Incremental Angle
    float inc   = mt6835.readIncAng(); 
     // Serial.print(">inc: ");     Serial.println(inc, 5); 

} 
  