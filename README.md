# eXo project 
 Qt frontend to MOC - [Webpage about eXo](http://loimu.tk/exo/)

### What it does ###
 Allows you to control your MOC player via tray popup menu.

### Build and run ###
```
#!Bash
mkdir build && cd build
cmake ..
make
./src/exo
```

### Building with Qt5 ###
```
#!Bash
mkdir build && cd build
cmake -DBUILD_WITH_QT4=OFF ..
make
./src/exo
```
NOTE: you should link against the Qt5 version of liblastfm or program will crash

### Build options: ###

```
#!
-DBUILD_WITH_QT4=OFF - build with Qt5 (set to Qt4 by default)
                       it's not working well with Status Notifier tray, will be fixed in Qt5.5 (I hope so);
-DBUILD_DBUS=OFF     - disable DBus and MPRISv2 interfaces (enabled by default);
-DBUILD_LASTFM=OFF   - disable scrobber module (enabled by default);
```


### Features: ###
* Scrobbling to last.fm (optional)
* OSD (additional Python script)
* Shows lyrics from web
* no GUI mode
* DBus interface (optional)
* initial MPRISv2 interface (optional)

### Screenshot ###
![exo-tray.png](https://bitbucket.org/repo/8Xb9ez/images/2886715694-exo-tray.png)

### Installation ###
For Ubuntu packages and instructions check the link above.
