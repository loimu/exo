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

#ifndef LYRICSPROVIDERS_H
#define LYRICSPROVIDERS_H

#include "qvector.h"

namespace LyricsProviders {
struct Provider {
    const QString name;
    const QString searchUrl;
    const QString urlTemplate;
    const QString urlRegExp;
    const QString dataRegExp;
    const QVector<QPair<QString, QString>> replaceList;
    const QVector<QString> excludeList;
};

extern const QVector<Provider> providers;
} // namespace LyricsProviders

#endif // LYRICSPROVIDERS_H