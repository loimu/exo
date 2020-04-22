# eXo project
 Qt frontend to MOC player

### Features: ###
* Scrobbling to last.fm (optional)
* Lyrics from web
* Background mode
* DBus and MPRISv2 interfaces (optional)
* Bookmarks
* OSD (additional Python script)

### Build and run ###
```bash
mkdir build && cd build
cmake ..
make -j2
./src/exo
```

### Build options: ###
```
-DBUILD_DBUS=OFF     - disable DBus and MPRISv2 interfaces (enabled by default);
-DBUILD_LASTFM=OFF   - disable scrobbler module (enabled by default);
-DBUILD_CMUS=ON      - enable Cmus interface (disabled by default);
-DBUILD_TRANSLATIONS=OFF - disable translations (enabled by default);
```

### Screenshot ###
![Screenshot of the eXo app](https://raw.githubusercontent.com/loimu/exo/master/assets/screenshot.png)

### Installation on Ubuntu ###
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

### Background mode (no GUI or X-session needed) ###
 Basic running: `exo -d 2>/tmp/exo_errors.log &`

 Scrobbler authentication: `exo -f` or `exo --force-reauth`

 Shutting down the app:
 
 `qdbus org.mpris.MediaPlayer2.exo /org/mpris/MediaPlayer2 org.mpris.MediaPlayer2.Quit`
 
 or just `pkill -2 exo`

### Tips ###
 Create a global hotkey to see the lyrics at any time.

 `qdbus local.exo_player /exo local.exo_player.showLyricsWindow` is the command you want.
