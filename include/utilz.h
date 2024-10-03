#ifndef UTILZ_H
#define UTILZ_H

#include <Arduino.h>
#include <vector>
#include <algorithm>

#ifndef LED_BUILTIN_PIN
    #define LED_BUILTIN_PIN 2
#endif

boolean ledBuiltinEnabled = false;

void setupLedBuiltin() {
    pinMode (LED_BUILTIN_PIN, OUTPUT);
    digitalWrite(LED_BUILTIN_PIN, LOW);
}

boolean ledBuiltinToggle() {
    ledBuiltinEnabled = !ledBuiltinEnabled;
    digitalWrite(LED_BUILTIN_PIN, ledBuiltinEnabled ? HIGH : LOW);
    return ledBuiltinEnabled;
}

void ledBuiltinOn() {
    ledBuiltinEnabled = false;
    ledBuiltinToggle();
}

void ledBuiltinOff() {
    ledBuiltinEnabled = true;
    ledBuiltinToggle();
}

/*
template <typename T>
int getIndex(std::vector<T> v, T* element) {
    auto it = std::find(v.begin(), v.end(), *element);

    if (it != v.end()) {
        return it - v.begin();
    } else {
        return -1;
    }
}
*/

template <typename T> 
const bool contains( std::vector<T>& v, const T& element ) {
    return std::find(v.begin(), v.end(), element) != v.end();
}

// https://stackoverflow.com/questions/31836598/subsampling-an-array-of-numbers
template <typename T>
void sampling(T* inBuffer, uint16_t IN, double* outBuffer, uint16_t OUT, double* iBuffer = NULL) {
    // std::array<int, OUT> res;
 
     for (int i = 0; i != OUT - 1; ++i) {
         int index = i * (IN - 1) / (OUT - 1);
         int p = i * (IN - 1) % (OUT - 1);
 
         outBuffer[i] = ((p * inBuffer[index + 1]) + (((OUT - 1) - p) * inBuffer[index])) / (OUT - 1);
         if (iBuffer != NULL) {
             iBuffer[i] = 0.0; // FFT Imaginary part must be zeroed in case of looping to avoid wrong calculations and overflows
         }
    }
    outBuffer[OUT - 1] = inBuffer[IN - 1]; // done outside of loop to avoid out of bound access (0 * a[IN])
    if (iBuffer != NULL) {
        iBuffer[OUT - 1] = 0.0;
    }
}

#endif