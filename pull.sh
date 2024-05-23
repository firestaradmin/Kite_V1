#!/bin/bash
versionS="v2.0"

echo "========================================"
echo "拉取远端对应到本地分支-懒人的脚本"
echo "Version $versionS @firestaradmin"
echo "========================================"


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
