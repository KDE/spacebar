# Spacebar Fakeserver

This is a test program to simulate the ModemManager DBus API and serve messages.

### How to use

Stop ModemManager:

```
$ sudo systemctl stop ModemManager
```

Run this program with elevated privileges (to use the system bus):

```
$ sudo spacebar-fakeserver
```