#!/bin/bash

# Modify this script for your own xmpp address / options :)

(while (( "$#" )); do
  echo -n "$1"
  shift 
done) | sendxmpp -i dst@test.com
