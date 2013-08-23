#include <QSettings>

#include "scrobblersettings.h"
#include "ui_scrobblersettings.h"

ScrobblerSettings::ScrobblerSettings(QSettings* settings) :
    ui(new Ui::ScrobblerSettings)
{
    ui->setupUi(this);
    m_settings = settings;
}

ScrobblerSettings::~ScrobblerSettings()
{
    delete ui;
}

void ScrobblerSettings::on_buttonBox_accepted() {
    if(ui->Username && ui->Password) {
        m_settings->setValue("scrobbler/enabled", true);
        m_settings->setValue("scrobbler/login", ui->Username->text());
        m_settings->setValue("scrobbler/password", ui->Password->text());
        m_settings->setValue("scrobbler/configured", true);
    }
    this->close();
}

void ScrobblerSettings::on_buttonBox_rejected() {
    m_settings->setValue("scrobbler/enabled", false);
    m_settings->setValue("scrobbler/configured", true);
    this->close();
}
