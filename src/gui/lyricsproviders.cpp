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

#include <QVector>

#include "lyricsproviders.h"

#define QSL QStringLiteral


const QVector<LyricsProviders::Provider> LyricsProviders::providers = {
    {
        QSL("chartlyrics.com"),
        QSL("http://www.chartlyrics.com/search.aspx?q=%1+%2"),
        QSL("http://www.chartlyrics.com%1"),
        QSL("<td>[^\\w]+<a href=\"([^\"]*)"),
        QSL("<p>(.*)</p>"),
        {{QSL("_@,;&\\/\""), QChar::fromLatin1('-')}},
        {{QSL("\r\n"), QString()}},
        {QSL("<img class=\"alignright\".*?/>")}
    },
    {
        QSL("elyrics.net"),
        QSL("https://www.elyrics.net/read/%3/%1-lyrics/%2-lyrics.html"),
        QString(), QString(),
        QSL("<div id='inlyr'>(.*)</div><br>"),
        {{QSL(" _@;&\\/\""), QChar::fromLatin1('-')}, {QSL("'"), QChar::fromLatin1('_')}},
        {{QSL("<br>"), QString()}},
        {QSL("<div.+div>")}
    },
    {
        QSL("songlyrics.com"),
        QSL("https://www.songlyrics.com/%1/%2-lyrics/"),
        QString(), QString(),
        QSL("<p id=\"songLyricsDiv\".*?\">(.*?)</p>"),
        {{QSL(" ._@,;&\\/\""), QChar::fromLatin1('-')}},
        {{QSL("\r\n"), QString()}, {QSL("<br />\n"), QSL("<br />")}}
    },
    {
        QSL("lyrics.ovh"),
        QSL("https://api.lyrics.ovh/v1/%1/%2"),
        QString(), QString(),
        QSL("{\"lyrics\":\"(.*)\"}"),
        {{QSL("_@,;&\\/\""), QChar::fromLatin1('-')}},
        {{QSL("\\n"), QSL("\n")}, {QSL("\\r"), QString()}, {QSL("\n\n"), QSL("\n")}}
    },
    {
        QSL("metal-archives.com"),
        QSL("https://www.metal-archives.com/search/ajax-advanced/searching/songs/"
            "?songTitle=%2&amp;bandName=%1&amp;ExactBandMatch=1"),
        QSL("https://www.metal-archives.com/release/ajax-view-lyrics/id/%1"),
        QSL("%1.*?lyricsLink_(\\d+)"),
        QSL("(.*)"),
        {{QSL("_@,;&\\\""), QChar::fromLatin1('-')}, {QSL("/"), QChar::Space}},
        {{QSL("\r\n"), QString()}}
    }
};
