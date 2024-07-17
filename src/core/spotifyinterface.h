/* ========================================================================
*    Copyright (C) 2013-2024 Blaze <blaze@vivaldi.net>
*
*    This file is part of eXo.
*
*    eXo is free software: you can redistribute it and/or modify
*    it under the terms of the GNU General Public License as published by
*    the Free Software Foundation, either version 3 of the License, or
*    (at your option) any later version.
*
*    eXo is distributed in the hope that it will be useful,
*    but WITHOUT ANY WARRANTY; without even the implied warranty of
*    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
*    GNU General Public License for more details.
*
*    You should have received a copy of the GNU General Public License
*    along with eXo.  If not, see <http://www.gnu.org/licenses/>.
* ======================================================================== */

#ifndef SPOTIFYINTERFACE_H
#define SPOTIFYINTERFACE_H

#include <QDBusInterface>

#include "playerinterface.h"

class QProcess;

/**
 * @brief The SpotifyInterface class
 * Aimed to be just an AudioScrobbler adaptor.
 * Spotify application should be responisble for the rest:
 * Lyrics, GUI and DBus interface functionality.
 */
class SpotifyInterface : public PlayerInterface
{
    const QString player;

    PState updateInfo() override;
    void timerEvent(QTimerEvent* event) override;

public:
    explicit SpotifyInterface(QObject* parent = nullptr);
    const QString id() const override final;
    void play() override final {};
    void pause() override final {};
    void playPause() override final {};
    void prev() override final {};
    void next() override final {};
    void stop() override final {};
    void quit() override final {};
    void jump(int) override final {};
    void seek(int) override final {};
    void volume(int) override {};
    void changeVolume(int) override {};
    void showPlayer() override {};
    void openUri(const QString&) override final {};
    void appendFile(const QStringList&) override final {};
    void clearPlaylist() override final {};
    void shutdown() override final {};
};

#endif // SPOTIFYINTERFACE_H
