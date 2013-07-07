#!/bin/bash

#Usage: rate.sh myPGN [anchor]
#runs pgnrate.tcl, which in turn runs bayeselo on myPGN pgnfile. 
#myPGN should NOT include .pgn extension. Anchor is the name of 
#a player to consider as reference for ELO
#
#assumes pgnrate and pgnstates are in current working directory
#pgnrate assumes bayeselo is in current working directory
if [ $# -eq 2 ] ; then

./pgnrate.tcl -anchor $2 $1.pgn
./pgnstats $1.pgn $2

else

./pgnrate.tcl $1.pgn
./pgnstats $1.pgn

fi
