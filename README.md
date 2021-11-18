# Spacebar

Spacebar is an SMS/MMS application that primarily targets Plasma Mobile.
It depends on Qt and few KDE Frameworks (Kirigami2, KI18n, KPeople, and ModemManagerQt).

# Architecture

Spacebar consists of an app and a daemon.
The app is user-facing, and only runs while the user is reading or writing messages. The daemon runs in the background to catch incoming SMS and MMS messages.

The database is mostly managed by the daemon, as it is responsible for writing incoming and outgoing messages into it.
It also sends notifications to the user when a new message arrived, using KNotifications.

The app connects to ModemManager to also get incoming messages, to display them live in the chat. It fetches the chat history from the database.
One other interaction with the database which the app does is marking messages as read, for the unread messages counter.
