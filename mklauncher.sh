#!/bin/sh

if [ ."$1" == . ] ; then
	echo "Syntax: mklauncher.sh <karlik-directory>" >&2
	exit 1
fi

echo "[Desktop Entry]"
echo "Version=1.0"
echo "Type=Application"
echo "Name=Karlík"
echo "Comment="
echo "Exec=$1/karlik"
echo "Icon=$1/img/appicon.png"
echo "Path=$1"
echo "Terminal=false"
echo "StartupNotify=false"
