# Line_follower
## NOTED POINTS FROM LFR

- ADC2 pins of esp32 will get blocked if BLE/WIFI is enabled
- getsensorvalues() function from qtr returns the raw analog value
  that value depends on the resolution of the ADC in the ESP
- QTR-8A is different from QTR-8RC, 8A can work only in typeAnalog mode
