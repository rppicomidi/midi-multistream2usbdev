# midi-multistream2usbdev

Modified version to work with Pico MIDI hat (link below) as a 6x6 MIDI interface using PIO MIDI...

https://github.com/diyelectromusic/sdemp_pcbs/tree/main/PicoMIDIRouter

![image](https://github.com/Ming-Kerr/midi-multistream2usbdev/assets/84568533/6e9e9b6e-adda-4e36-86cc-d235320ff4b7)
![image](https://github.com/Ming-Kerr/midi-multistream2usbdev/assets/84568533/070435ba-c6a8-42a4-bd95-9672ece8b1aa)

Build Instructions
This assumes you have installed the pico-sdk in ${PICO_SDK_PATH}.

cd ${PICO_SDK_PATH}/..

git clone https://github.com/Ming-Kerr/midi-multistream2usbdev

cd midi-multistream2usbdev

git submodule update --recursive --init

mkdir build

cd build

cmake -DCMAKE_BUILD_TYPE=Debug ..

make

