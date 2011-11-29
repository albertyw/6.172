#!/bin/sh

# This program simplifies the running of khetplayer 
# so that you don't have to input the setup commands 
# each time you want to run it

# Run using "./runkhet.sh" or "sh runkhet.sh"
# (you may need to first run "chmod 774 runkhet.sh" first)
cat khetcommands | ./khetplayer
