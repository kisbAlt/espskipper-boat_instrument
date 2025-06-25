#include <Arduino.h>
#include <transducerhandler.h>

const int pin_number = 12;

int TransducerHandler::ReadTemp() {
    int voltage_read = analogRead(pin_number) * (ANALOG_READ_REFERENCE_VOLTAGE/ANALOG_READ_RANGE); 
    
    double lnr =  log( (ANALOG_READ_REFERENCE_VOLTAGE * 10000.0 / voltage_read) - 10000.0 ); //get resistence of thermister
    int t = (int) (100 / (A + (B * lnr) + (C * lnr * lnr * lnr)) - 27315);  // Celsius
    
    return t;
}