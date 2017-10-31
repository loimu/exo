/* ========================================================================
*    Copyright (C) 2013-2017 Blaze <blaze@vivaldi.net>
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

#ifndef SCROBBLER_H
#define SCROBBLER_H

#include <QObject>

namespace lastfm {
    class Audioscrobbler;
}

class Scrobbler : public QObject
{
    Q_OBJECT

    static Scrobbler* object;
    lastfm::Audioscrobbler* as;
    void init(const QString& artist, const QString& title, int totalSec);
    void submit(const QString& artist, const QString& title,
                const QString& album, int totalSec);

public:
    static const char* apiKey;
    static const char* secret;
    explicit Scrobbler(QObject *parent = nullptr);
    ~Scrobbler();
    static Scrobbler* self() { return object; }
};

#endif // SCROBBLER_H
