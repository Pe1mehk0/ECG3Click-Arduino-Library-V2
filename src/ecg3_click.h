/* 
* MAX30003 Arduino Library 
* Ported and developed by Aliaksandr Kuryla  
* This library is released under the MIT License. 
* See the LICENSE file in the project root for full license information. 
*/

#ifndef ECG3_CLICK_H
#define ECG3_CLICK_H

#include <Arduino.h>
#include <SPI.h>

// --- General Configuration ---
#define _MAX30003_NO_OP_REG             0x00
#define _MAX30003_STAT_REG              0x01
#define _MAX30003_EN_INT_REG            0x02
#define _MAX30003_EN_INT2_REG           0x03
#define _MAX30003_MNGR_INT_REG          0x04
#define _MAX30003_MNGR_DYN_REG          0x05
#define _MAX30003_SW_RST_REG            0x08
#define _MAX30003_SYNC_REG              0x09
#define _MAX30003_FIFO_RST_REG          0x0A
#define _MAX30003_INFO_REG              0x0F
#define _MAX30003_CNFG_GEN_REG          0x10
#define _MAX30003_CNFG_CAL_REG          0x12
#define _MAX30003_CNFG_EMUX_REG         0x14
#define _MAX30003_CNFG_ECG_REG          0x15
#define _MAX30003_CNFG_RTOR1_REG        0x1D
#define _MAX30003_CNFG_RTOR2_REG        0x1E
#define _MAX30003_ECG_FIFO_BURST_REG    0x20
#define _MAX30003_ECG_FIFO_REG          0x21
#define _MAX30003_RTOR_REG              0x25

// --- Configuration Bits ---
#define _MAX30003_EINT_MASK             0x800000 
#define _MAX30003_EOVF_MASK             0x400000 
#define _MAX30003_FSTINT_MASK           0x200000 
#define _MAX30003_DCLOFF_INT_MASK       0x100000 
#define _MAX30003_LONINT_MASK           0x000800 
#define _MAX30003_RRINT_MASK            0x000400 
#define _MAX30003_SAMP_INT_MASK         0x000200 
#define _MAX30003_PLLINT_MASK           0x000100 

// General Config
#define _MAX30003_FMSTR_32768HZ_ECG_128HZ       0x200000 
#define _MAX30003_ECG_CHANN_EN                  0x080000 
#define _MAX30003_DCLOFF_EN                     0x001000 
#define _MAX30003_RBIAS_50M_OHM                 0x000000  
#define _MAX30003_RBIAS_100M_OHM                0x000004
#define _MAX30003_RBIAS_200M_OHM                0x000008  
#define _MAX30003_RBIASP_EN                     0x000002 
#define _MAX30003_RBIASN_EN                     0x000001 

// Calibration
#define _MAX30003_VCAL_EN                       0x400000 
#define _MAX30003_VMODE_BIPOL                   0x200000 
#define _MAX30003_VMAG_500MICROV                0x100000 

// MUX
#define _MAX30003_ECGP_EN                       0x000000 
#define _MAX30003_ECGN_EN                       0x000000 
#define _MAX30003_ECGP_CAL_VCALP                0x080000 
#define _MAX30003_ECGN_CAL_VCALN                0x030000 

// ECG Settings
#define _MAX30003_GAIN_40VPERV                  0x010000
#define _MAX30003_GAIN_20VPERV                  0x000000   
#define _MAX30003_DHPF_500MILIHZ                0x004000 
#define _MAX30003_DLPF_40HZ                     0x001000 

// RTOR Settings
#define _MAX30003_WNDW_12                       0x300000 
#define _MAX30003_RRGAIN_AUTO_SCALE             0x0F0000 
#define _MAX30003_RTOR_EN                       0x008000 
#define _MAX30003_PAVG_8                        0x002000 

// Commands
#define _MAX30003_SW_RST_CMD                    0x000000 
#define _MAX30003_FIFO_RST_CMD                  0x000000 
#define _MAX30003_SYNCH_CMD                     0x000000 

// --- Lead-Off Current Magnitude (IMAG) ---
#define _MAX30003_DCLOFF_IMAG_0NA        0x000000 
#define _MAX30003_DCLOFF_IMAG_5NA        0x000100 
#define _MAX30003_DCLOFF_IMAG_10NA       0x000200 
#define _MAX30003_DCLOFF_IMAG_20NA       0x000300 
#define _MAX30003_DCLOFF_IMAG_50NA       0x000400 
#define _MAX30003_DCLOFF_IMAG_100NA      0x000500 

// --- Lead-Off Voltage Threshold (VTH) ---
#define _MAX30003_DCLOFF_VTH_300MV       0x000000 
#define _MAX30003_DCLOFF_VTH_400MV       0x000040 
#define _MAX30003_DCLOFF_VTH_450MV       0x000080 
#define _MAX30003_DCLOFF_VTH_500MV       0x0000C0


class ECG3Click {
public:
    ECG3Click(int csPin);

    // Initialization
    bool begin(int sck, int miso, int mosi);

    // Core Data
    int32_t getECG();
    void getRTOR(uint16_t &hr, uint16_t &rr);
    
    // Status
    uint32_t getStatus();
    bool isLeadOff(int32_t currentVal);
    bool isRTORReady();

    // Register Access
    void writeRegister(uint8_t regAddr, uint32_t data);
    uint32_t readRegister(uint8_t regAddr);

private:
    int _csPin;
    SPISettings _spiSettings;
    
    void selectChip();
    void deselectChip();
    
    // Internal functions
    void ecg3_swReset();
    void ecg3_fifoReset();
    void ecg3_sync();
};

#endif