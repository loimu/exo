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
./exo
```

### Building with Qt5 ###
```
#!Bash
mkdir build && cd build
cmake -DBUILD_WITH_QT4=OFF -DLASTFM_LIBRARIES=/path/to/liblastfm.so ..  # path to Qt5 version of liblastfm
make
./exo
```
NOTE: you should link against the Qt5 version of liblastfm or program will crash

### Additional features: ###
* Scrobbling
* OSD
* Shows lyrics from web

### Screenshot ###
![exo-tray.png](https://bitbucket.org/repo/8Xb9ez/images/2886715694-exo-tray.png)

### Installation ###
For Ubuntu packages and instructions check the link above.