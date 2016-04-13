# Data-recorder
A C project for AVR to communicate with GNSS module, digital sensors and SD card.

It's a small project, which purpose is to record actual localization, temperature, humidity, pressure and altitude.
Sensors on board are MPL3115A2, HTS221TR and BMP280 (all digital).
The measurement of temperature and pressure is taken by few sensors to guarantee reliability.
Communication with SD card uses FatFS library.
To get localization GNSS module is used, which is working with GPS, GLONASS, GALILEO and other systems.
PCB board contains also DC-DC step down converter (+8VIN - 2xLiON to 3V3OUT) and programming slot.

Digital sensors are separated from rest devices, because in further project phase it is needed to isolate modules,
which shouldn't be exhibited on external weather conditions, from those, which should.

Above a scheme of this data recorder is presented:
