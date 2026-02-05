/* 
* MAX30003 Arduino Library 
* Ported and developed by Aliaksandr Kuryla  
* This library is released under the MIT License. 
* See the LICENSE file in the project root for full license information. 
*/

#include "ecg3_click.h"

ECG3Click::ECG3Click(int csPin) : _csPin(csPin) {}

bool ECG3Click::begin(int sck, int miso, int mosi) {
    pinMode(_csPin, OUTPUT);
    
    SPI.begin(sck, miso, mosi, _csPin);
    
    deselectChip();

    ecg3_swReset();
    delay(200);
    ecg3_fifoReset();
    ecg3_sync();

    // General Configuration
    writeRegister(_MAX30003_CNFG_GEN_REG, 
        _MAX30003_FMSTR_32768HZ_ECG_128HZ | 
        _MAX30003_ECG_CHANN_EN | 
        _MAX30003_DCLOFF_EN |
        _MAX30003_DCLOFF_IMAG_0NA |   
        _MAX30003_DCLOFF_VTH_500MV |
        _MAX30003_RBIAS_100M_OHM | 
        _MAX30003_RBIASP_EN | 
        _MAX30003_RBIASN_EN);

    // Calibration Settings
    writeRegister(_MAX30003_CNFG_CAL_REG, 0x000000);

    // Electrode Multiplexer Configuration
    writeRegister(_MAX30003_CNFG_EMUX_REG, 0x000000);

    // ECG-Specific Configuration
    writeRegister(_MAX30003_CNFG_ECG_REG, 
        0x805000 |                      
        _MAX30003_GAIN_40VPERV |            
        _MAX30003_DHPF_500MILIHZ | 
        _MAX30003_DLPF_40HZ);

    // RTOR Configuration
    writeRegister(_MAX30003_CNFG_RTOR1_REG, 
        _MAX30003_WNDW_12 | 
        _MAX30003_RRGAIN_AUTO_SCALE | 
        _MAX30003_RTOR_EN | 
        _MAX30003_PAVG_8 | 
        0x000600);

    ecg3_sync();

    // Read INFO Register
    uint32_t info = readRegister(_MAX30003_INFO_REG);
    if ((info & 0xF0) == 0x50) return true;
    
    return false;
}

void ECG3Click::writeRegister(uint8_t regAddr, uint32_t data) {
    selectChip();
    SPI.transfer((regAddr << 1) & 0xFE);
    SPI.transfer((data >> 16) & 0xFF);
    SPI.transfer((data >> 8) & 0xFF);
    SPI.transfer(data & 0xFF);
    deselectChip();
}

uint32_t ECG3Click::readRegister(uint8_t regAddr) {
    uint32_t result = 0;
    selectChip();
    SPI.transfer((regAddr << 1) | 0x01);
    result |= (SPI.transfer(0x00) << 16);
    result |= (SPI.transfer(0x00) << 8);
    result |= SPI.transfer(0x00);
    deselectChip();
    return result;
}

int32_t ECG3Click::getECG() {
    uint32_t rawECG = readRegister(_MAX30003_ECG_FIFO_REG);
    int32_t ecgData = ((int32_t)(rawECG << 8)) >> 14;
    return ecgData;
}

bool ECG3Click::isLeadOff(int32_t currentVal) {
    uint32_t status = getStatus();

    bool dcLeadOff = (status & 0x000008) != 0;
    bool refLeadOff = (status & 0x400000) && !(status & 0x000004);
    bool signalRailed = (currentVal > 35000 || currentVal < -35000 || currentVal == 0x7FFFFF);

    return (dcLeadOff || refLeadOff || signalRailed);
}


bool ECG3Click::isRTORReady() {
    uint32_t status = readRegister(_MAX30003_STAT_REG);
    return (status & _MAX30003_RRINT_MASK) ? true : false;
}

void ECG3Click::getRTOR(uint16_t &hr, uint16_t &rr) {
    uint32_t val = readRegister(_MAX30003_RTOR_REG);
    uint16_t rtor_ticks = (val >> 10) & 0x3FFF;
    float rr_ms = rtor_ticks * 8.0f; 
    
    if (rr_ms > 200.0f && rr_ms < 3000.0f) {
        rr = (uint16_t)rr_ms;
        hr = (uint16_t)(60000.0f / rr_ms);
    } else {
        rr = 0; hr = 0;
    }
}

void ECG3Click::ecg3_swReset() {
    writeRegister(_MAX30003_SW_RST_REG, _MAX30003_SW_RST_CMD);
    delay(10);
    Serial.println("ECG 3 Software Reset Complete.");
}

void ECG3Click::ecg3_fifoReset() {
    writeRegister(_MAX30003_FIFO_RST_REG, _MAX30003_FIFO_RST_CMD);
    Serial.println("ECG 3 FIFO Reset Complete.");
}

void ECG3Click::ecg3_sync() {
    writeRegister(_MAX30003_SYNC_REG, _MAX30003_SYNCH_CMD);
    Serial.println("ECG 3 Synchronization Complete.");
}

void ECG3Click::selectChip() {
    SPI.beginTransaction(SPISettings(2000000, MSBFIRST, SPI_MODE0));
    digitalWrite(_csPin, LOW);
}

void ECG3Click::deselectChip() {
    digitalWrite(_csPin, HIGH);
    SPI.endTransaction();
}

uint32_t ECG3Click::getStatus() {
    return readRegister(_MAX30003_STAT_REG);
}