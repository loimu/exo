/* ========================================================================
*    Copyright (C) 2013-2015 Blaze <blaze@open.by>
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

    void auth(const QString& username, const QString& password);
    lastfm::XmlQuery EmptyXmlQuery();
    bool parseQuery(const QByteArray& data, lastfm::XmlQuery* query,
                    bool* connectionProblems = NULL);
public:
    explicit ScrobblerAuth(QObject *parent = 0);

signals:
    void configured(bool);

private slots:
    void authReplyFinished();

public slots:
};

#endif // SCROBBLERAUTH_H
