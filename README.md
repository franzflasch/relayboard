relayboard
==========

Program for setting relais on Denkovi USB 4-Port relayboards.

Needs libftdi-dev.

Compiling:

make

make install

Usage:

relay [options]

Options:

 -l      list relay boards
 
 -o      device state to set can be (on) or (off) or (tog) for toggle
 
 -p      port, can be a number (1-4) or (all)
 
         if no option is given, all pins are switched
         
 -s      serial of relay board, find out with -l
 
 -v      verbose output
 
 
 
If no options are given, the program only prints the state of the relay.

This program is used to control an FTDI based relayboard.
I tested it with an Denkovi Ltd. 4-Port USB Relais board.
I have no idea if other boards work, too.
