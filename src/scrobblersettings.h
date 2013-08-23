#ifndef SCROBBLERSETTINGS_H
#define SCROBBLERSETTINGS_H

#include <QWidget>

class QSettings;

namespace Ui {
class ScrobblerSettings;
}

class ScrobblerSettings : public QWidget
{
    Q_OBJECT

public:
    explicit ScrobblerSettings(QSettings* settings);
    ~ScrobblerSettings();

private:
    Ui::ScrobblerSettings *ui;
    QSettings* m_settings;
private slots:
    void on_buttonBox_accepted();
    void on_buttonBox_rejected();
};

#endif // SCROBBLERSETTINGS_H
