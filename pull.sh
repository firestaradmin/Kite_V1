#!/bin/bash
echo "=================================="
echo "pull the newest note from github ..."
echo "=================================="
git fetch
git pull
echo " "
echo " "
echo "=================================="
git log -1 --format="[%h] %s"
echo "=================================="
echo "pull over, press any key to continue!"
read
#date=$(date)
#echo "\""$(date)"\""
