# Flash-BASH
Flash-BASH is a Raspberry Pi based tool that uses a multiplexer to "glitch" a target device into a privileged open state.

## Usage

See our blog post at https://www.riverloopsecurity.com/blog/2021/09/introducing-flash-bash/ for information on usage.

## Install/Compile
The easiest way to use this tool without any tweaks is with a Raspberry Pi Model 3 B+.
However, with minor tweaks to the code, any Raspberry Pi, Arduino, or micro-controller should work fine.

Secondly, this tool is based on the dependency Wiring Pi which you can install like so:
```
sudo apt-get install wiringpi
```

Third, you will need the Pi Hat and components that are included in the CAD folder in this repository. You could also wire this with jumpers and a breadboard or a prototype-hat, but it will not be as organized!

Last, you will need to compile this pointing to the Wiring Pi library like this
```
gcc -o flash_bash flash_bash.c -lwiringPi
```

That should get you started all you have to do now is run the program!
```
sudo ./flash_bash
```
