# MT6835-Encoder-Library
This Library lets you read the 21 bit absolute angle via SPI and 16 bit incremental angle via QEI.


Usage is very simple:

1) include your library header "MT6835.h".
2) Create the sensor object "MT6835 mt6835(...)". You can name it whatever you want. The inputs are ChipSelect Pin, SPI Frequency, Encoder Pin A, Encoder Pin B.
3) in "void.setup()" you just have to call the "initialize()" function.
4) to read the absolute angle call "readAbsAng()", for the incremental angle call "readIncAng()".

The file "Usage_example.cpp" shows how to use my library. If you found my code helpful, i would really appreciate it if you subscribed to my Youtube Channel! https://www.youtube.com/@SalimsWorkshop

Here is a video of me writing and explaining the sensor library: https://www.youtube.com/watch?v=dmIMhL8mpH8&lc=Ugx3C_RlMHlYo4ZAm1F4AaABAg.ANNCw_ojWXNANPgEevta1g
