# Spacebar

Spacebar is an SMS/MMS application that primarily targets Plasma Mobile.
It depends on Qt and few KDE Frameworks (Kirigami2, KI18n, KPeople, and ModemManagerQt).

# Architecture

Spacebar consists of an app and a daemon.
The app is user-facing, and only runs while the user is reading or writing messages. The daemon runs in the background to catch incoming SMS and MMS messages.

The database is mostly managed by the daemon, as it is responsible for writing incoming and outgoing messages into it.
It also sends notifications to the user when a new message arrived, using KNotifications.

The app also connects to the daemon via dbus to get incoming messages and display them live in the chat. It fetches the chat history from the database.
One other interaction with the database which the app does is marking messages as read, for the unread messages counter.

# Dependencies

## Required

```
Qt5Network
Qt5Qml
Qt5Quick
Qt5QuickControls2
Qt5Sql
Gettext
PkgConfig
PhoneNumber
Qt5Core
QCoro5

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

## Optional

```
QCoro5Coro
QCoro5Core
QCoro5DBus
```

# Build Instructions

+ Clone the repo with `git clone https://invent.kde.org/plasma-mobile/spacebar.git`
+ `cd` into the clone repo `cd spacebar`
+ Make a build directory with `mkdir build` and `cd` into it
+ Build with `cmake .. -DCMAKE_INSTALL_PREFIX=/path/to/install` then `cmake --build .` then `cmake --install .`
