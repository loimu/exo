/* ========================================================================
*    Copyright (C) 2013 Blaze <blaze@jabster.pl>
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

#include <lastfm/ws.h>
#include <lastfm/Audioscrobbler.h>
#include <lastfm/XmlQuery.h>

#include <QObject>

class QSettings;
class PlayerInterface;

class Scrobbler : public QObject
{
    Q_OBJECT

    void auth(const QString& username, const QString& password);
    lastfm::XmlQuery EmptyXmlQuery();
    bool ParseQuery(const QByteArray& data, lastfm::XmlQuery* query,
                    bool* connectionProblems = NULL);

    PlayerInterface* m_player;
    QSettings *m_settings;
    lastfm::Audioscrobbler* as;

public:
    Scrobbler(QObject *parent = 0, PlayerInterface *player = 0,
              QSettings *settings = 0);
private slots:
    void init();
    void submit();
    void authReplyFinished();
};

#endif // SCROBBLER_H
