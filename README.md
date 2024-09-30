# Working with SDR receiver
### Program for getting data from RTL-SDR receiver and plotting a spectrum graph from obtained data

![image](https://i.imgur.com/rPifpfD.png)

## Description
Initially this program was developed for the department of radiophysics of Kazan Federal University. RTL-SDR received waves from meteoradar reflected from meteor trails. I publish the program source code for educational purposes as an example of usage Soapy-SDR, gnuplot-iostream, fftw3 and argp libraries.

## Usage

Without arguments program starts to collect iq-values from the RTL-SDR receiver. With the `-p FILE_NAME.iq` argument and program will plot the spectrum of saved signal. Use `radar --usage` for more info.

## Building

You need to install [SoapyRTLSDR](https://github.com/pothosware/SoapyRTLSDR) and its dependencies and make sure you have all of the following packages before compiling:
 - cmake
 - g++ or clang compiler
 - libfftw3-dev
 - libgnuplot-iostream-dev

Clone this repo and run these commands:
```
$ mkdir build
$ cd build
$ cmake ..
$ cmake --build .
```

## Was this project useful?
:star2: You can always give a star on the project to say thanks

*OR*

:moneybag: You can donate me:
> BTC: bc1q8rcdhq4pfn45pf647afhjtdn6qhrheu3hngn9m

:heart: You can always offer me a real job and I will do useful things instead of this one