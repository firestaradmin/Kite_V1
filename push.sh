#!/bin/bash
versionS="V1.3"

echo "=================================="
echo "pushing note to git repository ..."
echo "Version $versionS"
echo "=================================="

git add .
git status

if [ ! $1 ]; then
	echo "please input commit or press Enter to skip:"
	read commitS
	if [ ! $commitS ]; then
		# default opreation
		git commit -m "$(date) with push.sh $versionS"
		# echo "$(date) with push.sh"
	else
		git commit -m "$commitS | with push.sh $versionS"
		# echo "$commitS | with push.sh"
	fi
else
	git commit -m "$1 | with push.sh $versionS"
	# echo "$1 | with push.sh"
fi


git push
echo " "
echo " "
echo "=================================="
git log -1 --format="[%h] %s"
echo "=================================="
echo "upload over, press any key to continue!"
read
#date=$(date)
#echo "\""$(date)"\""
