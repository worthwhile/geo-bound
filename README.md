# Geo-Bound

* records GPS location and sends to [particle cloud](https://www.particle.io/)
* designed to work with Photon + Asset Tracker Kit v2
* works with no cell service saving up to 2000 readings
* sends readings when cell becomes available

## Getting the tools needed to compile

Assuming you have the latest xcode and homebrew installed,

- brew install dfu-util
- sudo npm install -g particle-cli
- particle login


## Project Structure

Every Particle project is composed of 3 important elements that you'll see have been created in your project directory for geo-bound.

#### ```/src``` folder:  
This is the source folder that contains the firmware files for your project. It should *not* be renamed. 
Anything that is in this folder when you compile your project will be sent to our compile service and compiled into a firmware binary for the Particle device that you have targeted.

If your application contains multiple files, they should all be included in the `src` folder. If your firmware depends on Particle libraries, those dependencies are specified in the `project.properties` file referenced below.

#### ```.ino``` file:
This file is the firmware that will run as the primary application on your Particle device. It contains a `setup()` and `loop()` function, and can be written in Wiring or C/C++. For more information about using the Particle firmware API to create firmware for your Particle device, refer to the [Firmware Reference](https://docs.particle.io/reference/firmware/) section of the Particle documentation.

#### ```project.properties``` file:  
This is the file that specifies the name and version number of the libraries that your project depends on. Dependencies are added automatically to your `project.properties` file when you add a library to a project using the `particle library add` command in the CLI or add a library in the Desktop IDE.

## Adding additional files to your project

#### Projects with multiple sources
If you would like add additional files to your application, they should be added to the `/src` folder. All files in the `/src` folder will be sent to the Particle Cloud to produce a compiled binary.

#### Projects with external libraries
If your project includes a library that has not been registered in the Particle libraries system, you should create a new folder named `/lib/<libraryname>/src` under `/<project dir>` and add the `.h` and `.cpp` files for your library there. All contents of the `/lib` folder and subfolders will also be sent to the Cloud for compilation.

## Compiling your project

When you're ready to compile your project, make sure you have the correct Particle device target selected and run `particle compile <platform>` in the CLI or click the Compile button in the Desktop IDE. The following files in your project folder will be sent to the compile service:

- Everything in the `/src` folder, including your `.ino` application file
- The `project.properties` file for your project
- Any libraries stored under `lib/<libraryname>/src`

**To compile the firmware:**

- cd GPSTracker 
- particle compile electron . --saveTo firmware.bin

This will compile the files in the current directory and create firmware.bin for flashing.

## Flashing your project

First, attach the micro USB to the particle, and plug it into your computer.

Make sure you are in the **geo-bound** folder, where your firmware.bin resides.

If you have a brand new electron, its **system** firmware will be outdated.  The electron has a system firmware and project 
firmware and these are flashed separately. 

**DFU Mode**

In order to flash the electron, it must be in DFU mode. To put the electron into DFU mode position the electron so the usb connector is pointing away from you.  
Press both buttons on the particle at the same time, then release the button on the right. 

Keep holding the left button until the breathing light starts flashing yellow.  When it is flashing yellow, the electron is in 
DFU mode and is ready to be flashed.

**To update the firmware, run the following:**
- (Enable DFU Mode)
- particle update

**To flash the firmware**
- (Enable DFU Mode)
- particle flash --usb firmware.bin


## Debugging

Use **particle serial monitor** while connected to usb cable to see what is going on.  You can put Serial.printlnf statements in your code and you'll 
see the output on the serial monitor.

