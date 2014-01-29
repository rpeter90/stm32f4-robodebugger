RoboDebugger 
=============
(for STM32F4 microcontrollers)

Simple command line based debugger for STM32F4 microcontrollers, such as STM32F407 and STM32F429, written in C language.

Current version: 0.1
Current test build: 1 (STM32F407VGT6 - Discovery Board)
Programming enviroment: Eclipse Kepler
Compiler: GNU Tools for ARM Embedded Processors 4.8 2013q4
Project started: 2014.01.29. 
Estimated finish: 2014.03.15.

Features (already done)
-----------------------

- Communication over USART3 channel, only in asynchronous mode (Tx/Rx).
- Basic command interpreter (using space and \r for command recognition).
- Extensible command interpreter, with programmable commands (using a simple structure for each command).
- Extensible error list, and error handling.
- Easy configuration in header file (only buffer parameters, command and error numbers, argument numbers and such).

Planned Features
----------------

- Communication over multiple channels, and multiple ways (I2C, USART, SPI, based on configuration).
- Configurable command interpreter.
- Extensive command definitions (such as help features and argument checking).
- Separate config file.

