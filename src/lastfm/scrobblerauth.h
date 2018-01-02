/* ========================================================================
*    Copyright (C) 2013-2018 Blaze <blaze@vivaldi.net>
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

#ifndef SCROBBLERAUTH_H
#define SCROBBLERAUTH_H

#include <QObject>

class QSettings;

namespace lastfm {
class XmlQuery;
}

class ScrobblerAuth : public QObject
{
    Q_OBJECT

    lastfm::XmlQuery EmptyXmlQuery();
    bool parseQuery(const QByteArray& data, lastfm::XmlQuery* query,
                    bool* connectionProblems = nullptr);
    void authReplyFinished();

public:
    explicit ScrobblerAuth(QObject *parent = nullptr);
    void auth(const QString& username, const QString& password);

signals:
    void configured();
    void failed(const QString& errmsg);
};

#endif // SCROBBLERAUTH_H
