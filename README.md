# Raspberry Pi Pico 6in 6out MIDI Interface using both hardware UARTS and 4xPIO UARTS

Code built to work with Pico MIDI Routing hat (link below) as a 6x6 MIDI interface using both PIO MIDI and hardware UARTS...

https://github.com/diyelectromusic/sdemp_pcbs/tree/main/PicoMIDIRouter

![image](https://github.com/Ming-Kerr/midi-multistream2usbdev/assets/84568533/6e9e9b6e-adda-4e36-86cc-d235320ff4b7)
![image](https://github.com/Ming-Kerr/midi-multistream2usbdev/assets/84568533/070435ba-c6a8-42a4-bd95-9672ece8b1aa)

Build Instructions
This assumes you have installed the pico-sdk in ${PICO_SDK_PATH}.
Will also need midi_uart_lib in lib folder.

cd ${PICO_SDK_PATH}/..

git clone https://github.com/Ming-Kerr/midi-multistream2usbdev

cd midi-multistream2usbdev

git submodule update --recursive --init

mkdir build

cd build

cmake -DCMAKE_BUILD_TYPE=Debug ..

make

