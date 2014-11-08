# eXo project 
 Qt frontend to MOC - [Webpage about eXo](http://loimu.tk/exo/)

## What it does ##
 Allows you to control your MOC player via tray popup menu.

## Build and run ##
* cmake .
* make
* ./exo

NOTE: when building with Qt5 link against the Qt5 version of liblastfm
* -DBUILD_WITH_QT4=OFF -DLASTFM_LIBRARIES=/path/to/liblastfm.so

## Additional features: ##
* Scrobbling
* OSD
* Shows lyrics from web

## Screenshot ##
![exo-tray.png](https://bitbucket.org/repo/8Xb9ez/images/2886715694-exo-tray.png)