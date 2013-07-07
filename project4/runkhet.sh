#!/bin/sh

# This program simplifies the running of khetplayer 
# so that you don't have to input the setup commands 
# each time you want to run it

# Feel free to change the commands within the 
# khetcommands file to suit your testing needs

# Run using "./runkhet.sh" or "sh runkhet.sh"
# (you may need to first run "chmod 774 runkhet.sh" first)
cat khetcommands | ./khetplayer
