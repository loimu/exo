# eXo project
 Qt frontend to MOC - [Webpage about eXo](https://weblog-loimu.rhcloud.com/exo/)

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
NOTE: you should link against the corresponding version of liblastfm or program will crash

### Build options: ###

```
#!
-DBUILD_DBUS=OFF     - disable DBus and MPRISv2 interfaces (enabled by default);
-DBUILD_LASTFM=OFF   - disable scrobber module (enabled by default);
```


### Features: ###
* Scrobbling to last.fm (optional)
* Lyrics from web
* Background mode
* DBus and MPRISv2 interfaces (optional)
* Bookmarks
* OSD (additional Python script)

### Screenshot ###
![Screenshot of the eXo app](https://bitbucket.org/repo/8Xb9ez/images/405669011-exo.png)

### Installation ###
For Ubuntu packages and instructions check the link above.
