#!/bin/bash

pip install PyInstaller

pyinstaller -n "TimeTracker" --add-data "assets:assets" main.py

mkdir -p package/opt


mkdir -p package/opt
mkdir -p package/usr/share/applications
mkdir -p package/usr/share/icons/hicolor/scalable/apps
cp -r dist/TimeTracker package/opt

touch package/usr/share/applications/TimeTracker.desktop

echo "[Desktop Entry]

# The type of the thing this desktop file refers to (e.g. can be Link)
Type=Application

# The application name.
Name=Time Tracker

# Tooltip comment to show in menus.
Comment=Track your working time.

# The path (folder) in which the executable is run
Path=/opt/TimeTracker

# The executable (can include arguments)
Exec=/opt/TimeTracker/TimeTracker

# The icon for the entry, using the name from hicolor/scalable without the extension.
# You can also use a full path to a file in /opt.
Icon=TimeTracker
" > package/usr/share/applications/TimeTracker.desktop