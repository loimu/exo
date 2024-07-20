# eXo project
 Qt frontend to MOC (Music on Console) player.
 Combination of eXo and MOC gives you a modern yet very minimalistic audio player.

## Features: ##
* Lyrics from web
* Background mode
* DBus and MPRISv2 interfaces. Enables media controls in your desktop environment
* Scrobbling to last.fm (optional)
* Bookmarks for radio streams
* OSD (additional Python script)
* Optional CMus support
* Optional Spotify client support (limited functionality, scrobbling only)

See `man exo` for more information on command line options.

## Build dependencies ##
CMake, QtBase, QtTools, LibLastfm (optional)

```bash
sudo apt install qtbase5-dev qttools5-dev liblastfm5-dev cmake
```

## Runtime dependencies ##
moc, python3-notify2, cmus(?), spotify(?)

## Build and run ##
```bash
mkdir build && cd build
cmake -DBUILD_LASTFM=ON ..
make -j2
./src/exo
```

## Build options: ##
```
BUILD_LASTFM=OFF # disable scrobbler module (enabled by default)
WITH_QT6=ON      # build with Qt6 (disabled by default)
```

## Screenshot ##
![Screenshot of the eXo app](https://raw.githubusercontent.com/loimu/exo/master/assets/screenshot.png)

## Installation on Ubuntu ##
```bash
# stable releases
sudo add-apt-repository ppa:blaze/main
sudo apt update
sudo apt install exo-player

# development snapshots
sudo add-apt-repository ppa:blaze/dev
sudo apt update
sudo apt install exo-player
```

## Background mode (no GUI or X-session needed) ##
 ### Basic running ###
    exo -d 2>/tmp/exo_errors.log &

 ### Scrobbler (re)authentication ###
    exo --force-reauth

 ### Shutting down the app ###
```bash
qdbus org.mpris.MediaPlayer2.exo\
  /org/mpris/MediaPlayer2 org.mpris.MediaPlayer2.Quit
```
or

    pkill -2 exo

## Tips ##
 ### Create a global hotkey to see the lyrics at any time by assigning the command below to a desired shortcut ###
    qdbus local.exo_player /exo local.exo_player.showLyricsWindow

### Trick eXo into running without DBus and MPRISv2 interfaces, if needed, by resetting DISPLAY variable ###
    DISPLAY="" exo &
