<!--
- SPDX-FileCopyrightText: None
- SPDX-License-Identifier: CC0-1.0
-->

# Spacebar <img src="logo.png" width="40"/>
Spacebar is a SMS/MMS messaging client. It allows you to send text messages, pictures and other files over a cellular network.

## Get it
A stable release [is available](https://apps.kde.org/spacebar) for download for Linux distributions.

## Links
* Project page: https://invent.kde.org/plasma-mobile/spacebar
* File issues: https://bugs.kde.org/describecomponents.cgi?product=spacebar
* Development channel: https://matrix.to/#/#plasmamobile:matrix.org

## Building and Installing
```sh
git clone https://invent.kde.org/plasma-mobile/spacebar.git
cd spacebar
cmake -B build && cmake --build build
sudo cmake --install build
```

## Dependencies
```
Qt5Qml
Qt5Quick
Qt5QuickControls2
Qt5Sql
Gettext
PkgConfig
PhoneNumber
Qt5Core
QCoro5
CURL
c-ares

(Required version >= 5.88.0)
ECM
KF5Kirigami2
KF5I18n
KF5People
KF5Notifications
KF5Config
KF5CoreAddons
KF5DBusAddons
KF5ModemManagerQt
KF5

(Required version >= 5.15.2)
Qt5QmlModels
Qt5Gui
Qt5Widgets
Qt5Xml

(Required version >= 5.15.0)
Qt5
```

## Components
Spacebar is split into two components: **spacebar** (front-end), and **spacebar-daemon** (backend-daemon).

### spacebar-daemon
The background daemon, which is configured to autostart, has the following responsibilities:
* Handles the sending and receiving of messages.
* Saves messages to the database.
* Creates notifications when a new message arrives.

`spacebar-daemon` exposes its API in D-Bus under the service name `org.kde.spacebar.Daemon`. Front-end applications like `spacebar` depend on it for messaging functionality. It manages incoming messages and notifications in the background without needing a front-end app running.

### spacebar
A front-end app for spacebar-daemon written in Kirigami.

Run with these environment variables to have mobile controls:
```
QT_QUICK_CONTROLS_MOBILE=true spacebar
```
