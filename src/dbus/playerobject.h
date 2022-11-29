/* ========================================================================
*    Copyright (C) 2013-2022 Blaze <blaze@vivaldi.net>
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

#ifndef PLAYEROBJECT_H
#define PLAYEROBJECT_H

#include <QDBusAbstractAdaptor>
#include <QDBusObjectPath>
#include <QObject>


class PlayerInterface;
enum class PlayerState;
using PState = PlayerState;

// clazy:excludeall=qproperty-without-notify
class PlayerObject : public QDBusAbstractAdaptor
{
    Q_OBJECT
    Q_CLASSINFO("D-Bus Interface", "org.mpris.MediaPlayer2.Player")
    Q_PROPERTY(bool CanControl READ canControl CONSTANT)
    Q_PROPERTY(bool CanGoNext READ canGoNext CONSTANT)
    Q_PROPERTY(bool CanGoPrevious READ canGoPrevious CONSTANT)
    Q_PROPERTY(bool CanPause READ canPause CONSTANT)
    Q_PROPERTY(bool CanPlay READ canPlay CONSTANT)
    Q_PROPERTY(bool CanSeek READ canSeek)
    Q_PROPERTY(QVariantMap Metadata READ metadata)
    Q_PROPERTY(QString PlaybackStatus READ playbackStatus)
    Q_PROPERTY(qlonglong Position READ position)
    Q_PROPERTY(double Volume MEMBER volume WRITE setVolume)

    PlayerInterface* player = nullptr;
    double volume = 0.5;
    PState status;
    QVariantMap props;
    QString cover;
    QDBusObjectPath trackID;
    void syncProperties();
    void trackChanged(const QString&);
    void emitPropsChanged(PState state);

public:
    explicit PlayerObject(QObject* parent = nullptr);
    bool canControl() const;
    bool canGoNext() const;
    bool canGoPrevious() const;
    bool canPause() const;
    bool canPlay() const;
    bool canSeek() const;
    QVariantMap metadata() const;
    QString playbackStatus() const;
    qlonglong position() const;
    void setVolume(double value);

public Q_SLOTS:
    void Next();
    void Pause();
    void Play();
    void PlayPause();
    void Previous();
    void Stop();
    void Seek(qlonglong Offset);
    void SetPosition(const QDBusObjectPath &TrackId, qlonglong Position);
    void OpenUri(const QString &Uri);
};

#endif // PLAYEROBJECT_H
