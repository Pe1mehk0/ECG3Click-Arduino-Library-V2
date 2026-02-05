#include "ecg3_click.h"

// Your Pins
#define SCK_PIN   5
#define MISO_PIN  21
#define MOSI_PIN  19
#define CS_PIN    15

ECG3Click ecg(CS_PIN);

void setup() {
    Serial.begin(115200);
    while(!Serial);

    if (ecg.begin(SCK_PIN, MISO_PIN, MOSI_PIN)) {
        Serial.println("INIT SUCCESS");
    } else {
        Serial.println("INIT FAILED");
    }
}

void loop() {
    // Check Status
    uint32_t status = ecg.getStatus();

    // Check if new ECG sample is ready
    if (status & _MAX30003_EINT_MASK) {
        
        // Read ECG value
        int32_t ecg_val = ecg.getECG();
        
        // Print Start of Packet with ECG
        Serial.print("E:");
        Serial.print(ecg_val);

        // Check if Heart Rate & RTOR is ready
        if (ecg.isRTORReady()) {
            uint16_t hr, rr;
            ecg.getRTOR(hr, rr);
            
            Serial.print(",H:");
            Serial.print(hr);
            Serial.print(",R:");
            Serial.print(rr);
        }

        Serial.println();
    }
    
    // Tiny delay to keep Serial stable
    delayMicroseconds(50);
}