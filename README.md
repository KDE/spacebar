# Spacebar

Telepathy client for Plasma Mobile, that is primarily useful for sending and receiving SMS.

# Development setup

Spacebar requires an ofono based telepathy account to be set up.
While in most Plasma Mobile images this should be set up out of the box, it requires some manual work on a development machine.

Make sure to install the [telepathy-ofono](https://github.com/TelepathyIM/telepathy-ofono) backend.

Then, set up ofono-phonesim according to [docs.plasma-mobile.org/Ofono.html](https://docs.plasma-mobile.org/Ofono.html#phonesim), to add a fake modem.

As a last step copy the [telepathy mission-control accounts configuration file](https://raw.githubusercontent.com/KDE/plasma-phone-settings/master/etc/skel/.local/share/telepathy/mission-control/accounts.cfg)
to `.local/share/telepathy/mission-control/accounts.cfg`.
