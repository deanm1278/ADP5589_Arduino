/* ADP5589.cpp
 * 
 * Arduino library for the ADP5589 keypad controller
 * April 7, 2016 Dean Miller
 * MIT license
 */

//not tested with this little guy
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

void ADP5589::createEventMatrix(void){
    //read the active rows and columns
    uint8_t activeRows = readRegister(ADP5589_ADR_PIN_CONFIG_A);
    uint8_t activeColumns = readRegister(ADP5589_ADR_PIN_CONFIG_B);
    
    //free all active events
    for(int i=0; i<numEvents; i++){
        free(eventMatrix[i]);
    }
    numEvents = 0;
    
    for(int i=0; i<7; i++){
        //if this row is active, set the event active on all active columns
        if((activeRows >> i) & 1){
            for(int j=0; j<7; j++){
                if((activeColumns >> j) & 1){
                    event *evt = (event *)malloc(sizeof(struct event));
                    evt->number = getEvent(i, j);
                    evt->status = false;
                    eventMatrix[numEvents] = evt;
                    numEvents++;
                }
            }
        }
    }
}

void ADP5589::update(void){
    int newEvents = readRegister(ADP5589_ADR_STATUS) & 0x1F;
    
    //update all events from the last read
    for(int i=0; i<numEvents; i++){
        event *evt = eventMatrix[i];
        if(evt->status == ADP5589_RISING)
            evt->status = ADP5589_HIGH;
        else if(evt->status == ADP5589_FALLING)
            evt->status = ADP5589_LOW;
    }
    
    //read all events in the fifo
    for(int i=0; i<newEvents; i++)
    {
    	uint8_t evt = readRegister(ADP5589_ADR_FIFO1);
        uint8_t evtNumber = evt & 0x7F;
        bool evtStatus = evt >> 7;
        
        //update the event matrix
        for(int j=0; j<numEvents; j++){
            event *e = eventMatrix[j];
            if(e->number == evtNumber){
                if(evtStatus)
                    e->status = ADP5589_RISING;
                else
                    e->status = ADP5589_FALLING;
            }
        }
    }
    
    //call any callbacks
    for(int i=0; i<numCallbacks; i++){
        callback *cb = callbacks[i];
        event *evt = eventMatrix[cb->evtIndex];
        if(cb->callbackType == evt->status)
            cb->callback();
    }
}

bool ADP5589::registerCallback(uint8_t row, uint8_t col, int (*fn)(void), uint8_t type){
    uint8_t evtNumber = getEvent(row, col);
    for(int j=0; j<numEvents; j++){
        event *e = eventMatrix[j];
        if(e->number == evtNumber){
            callback *cb = (callback *)malloc(sizeof(struct callback));
            cb->callbackType = type;
            cb->evtIndex = j;
            cb->callback = fn;
            callbacks[numCallbacks] = cb;
            numCallbacks++;
            break;
        }
    }
}

void ADP5589::activateRow(uint8_t row){
    if(row <= 7){
        writeRegister(ADP5589_ADR_PIN_CONFIG_A, (1 << row) | readRegister(ADP5589_ADR_PIN_CONFIG_A));
        createEventMatrix();
    }
}

void ADP5589::activateColumn(uint8_t col){
    if(col <= 7){
        writeRegister(ADP5589_ADR_PIN_CONFIG_B, (1 << col) | readRegister(ADP5589_ADR_PIN_CONFIG_B));
        createEventMatrix();
    }
}

void ADP5589::gpioSetDirection(uint8_t gpio, uint8_t dir){
    if(gpio > 0 && gpio <= 19 && dir >= 0 && dir <= 1){
        uint8_t bit = ((gpio - 1) % 8);
        uint8_t reg = gpio_dir_reg[(gpio - 1) / 8];
        
        uint8_t toWrite = readRegister(reg);
        toWrite ^= (-dir ^ toWrite) & (1 << bit);
        
        writeRegister(reg, toWrite);
    }
}

void ADP5589::writeRegister(uint8_t reg, uint8_t val){
    //Write the specified value to the specified register
    Wire.beginTransmission(i2c_addr);
    Wire.write(reg); //set register pointer
    Wire.write(val); //set value
    Wire.endTransmission();
}

void ADP5589::gpioWrite(uint8_t gpio, uint8_t val){
    //Write the specified value to the specified GPIO pin
    if(gpio > 0 && gpio <= 19 && val >= 0 && val <= 1){
        uint8_t bit = ((gpio - 1) % 8);
        uint8_t reg = gpo_data_reg[(gpio - 1) / 8];
        
        uint8_t toWrite = readRegister(reg);
        toWrite ^= (-val ^ toWrite) & (1 << bit);
        
        writeRegister(reg, toWrite);
    }
}

uint8_t ADP5589::readRegister(uint8_t reg){
    uint8_t resp;
    
    Wire.beginTransmission(i2c_addr);
    Wire.write(reg); //set the register pointer
    Wire.endTransmission();
    
    //read back the response
    uint8_t num_bytes = 1;
    Wire.requestFrom(i2c_addr, num_bytes);

    while(Wire.available())
    { 
      resp = Wire.read();
    }
    
    return resp;
}

uint8_t ADP5589::getEvent(uint8_t row, uint8_t col){
    return 1 + (row * ADP5589_NUM_COLUMNS) + col;
}