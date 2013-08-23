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
                    bool* connection_problems = NULL);
    PlayerInterface* m_player;
    QSettings* m_settings;
    lastfm::Audioscrobbler* as;
public:
    Scrobbler(QObject *parent = 0, QSettings *settings = 0, PlayerInterface *player = 0);
private slots:
    void init();
    void submit();
    void authReplyFinished();
signals:
    void AuthenticationComplete(bool);
};

#endif // SCROBBLER_H
