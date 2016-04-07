/* ADP5589.cpp
 * 
 * Arduino library for the ADP5589 keypad controller
 * April 7, 2016 Dean Miller
 * TODO: look up licenses
 */

#ifdef __AVR_ATtiny85__
  #include <TinyWireM.h>
  #define Wire TinyWireM
#else
  #include <Wire.h>
#endif

#include "ADP5589.h"

static const uint8_t gpio_dir_reg[] = {
    ADP5589_ADR_GPIO_DIRECTION_A,
    ADP5589_ADR_GPIO_DIRECTION_B,
    ADP5589_ADR_GPIO_DIRECTION_C
};

static const uint8_t gpo_data_reg[] = {
    ADP5589_ADR_GPO_DATA_OUT_A,
    ADP5589_ADR_GPO_DATA_OUT_B,
    ADP5589_ADR_GPO_DATA_OUT_C
};

ADP5589::ADP5589(void){
}

void ADP5589::begin(uint8_t addr){
    i2c_addr = addr;
    
    Wire.begin();
    
    //enable the internal oscillator, set to 500khz
    writeRegister(ADP5589_ADR_GENERAL_CFG_B, ADP5589_GENERAL_CFG_B_OSC_EN| 
                             ADP5589_GENERAL_CFG_B_CORE_FREQ(3));
}

void ADP5589::activateRow(uint8_t row){
    if(row <= 7)
        writeRegister(ADP5589_ADR_PIN_CONFIG_A, (1 << row));
}

void ADP5589::activateColumn(uint8_t col){
    if(col <= 7)
        writeRegister(ADP5589_ADR_PIN_CONFIG_A, (1 << col));
}

void ADP5589::gpioSetDirection(uint8_t gpio, uint8_t dir){
    if(gpio > 0 && gpio <= 19 && dir >= 0 && dir <= 1){
        uint8_t bit = ((gpio - 1) % 8);
        uint8_t reg = gpio_dir_reg[(gpio - 1) / 8];

        writeRegister(reg, (dir << bit));
    }
}

void ADP5589::writeRegister(uint8_t reg, uint8_t val){
    Wire.beginTransmission(i2c_addr);
    Wire.write(reg);
    Wire.write(val);
    Wire.endTransmission();
}

void ADP5589::gpioWrite(uint8_t gpio, uint8_t val){
    if(gpio > 0 && gpio <= 19 && val >= 0 && val <= 1){
        uint8_t bit = ((gpio - 1) % 8);
        uint8_t reg = gpio_dir_reg[(gpio - 1) / 8];

        writeRegister(reg, (val << bit));
    }
}

uint8_t ADP5589::readRegister(uint8_t reg){
    uint8_t resp;
    
    Wire.beginTransmission(i2c_addr);
    Wire.write(reg); //set the register pointer
    Wire.endTransmission();
    
    //read back the response
    uint8_t num_bytes = 1;
    Wire.requestFrom(num_bytes, i2c_addr);

    while(Wire.available())
    { 
      resp = Wire.read();
    }
    
    return resp;
}