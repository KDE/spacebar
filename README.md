# Spacebar

Spacebar is an ofono based SMS application that primarily targets Plasma Mobile.
It depends on Qt, a few KDE Frameworks (Kirigami2, KI18n, KPeople and KContacts) and libqofono.

For development, you need `libqofono` and [a running ofono-phonesim](https://docs.plasma-mobile.org/Ofono.html).

# Architecture

Spacebar consists of an app and a daemon.
The app is user-facing, and only runs while the user is reading or writing messages. The daemon runs in the background to catch incoming SMS.

The database is mostly managed by the daemon, as it is responsible for writing incoming and outgoing messages into it.
It also sends notifications to the user when a new message arrived, using KNotifications.

The app connects directly to ofono to also get incoming messages, to display them live in the chat. It fetches the chat history from the database.
One other interaction with the database which the app does is marking messages as read, for the unread messages counter.
