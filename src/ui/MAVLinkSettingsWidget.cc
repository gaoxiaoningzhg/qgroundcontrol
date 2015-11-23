/*=====================================================================

QGroundControl Open Source Ground Control Station

(c) 2009, 2010 QGROUNDCONTROL PROJECT <http://www.qgroundcontrol.org>

This file is part of the QGROUNDCONTROL project

    QGROUNDCONTROL is free software: you can redistribute it and/or modify
    it under the terms of the GNU General Public License as published by
    the Free Software Foundation, either version 3 of the License, or
    (at your option) any later version.

    QGROUNDCONTROL is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
    GNU General Public License for more details.

    You should have received a copy of the GNU General Public License
    along with QGROUNDCONTROL. If not, see <http://www.gnu.org/licenses/>.

======================================================================*/

/**
 * @file
 *   @brief Implementation of MAVLinkSettingsWidget
 *   @author Lorenz Meier <mail@qgroundcontrol.org>
 */

#include <QFileInfo>
#include <QStandardPaths>

#include "MAVLinkSettingsWidget.h"
#include "LinkManager.h"
#include "UDPLink.h"
#include "QGCApplication.h"
#include "ui_MAVLinkSettingsWidget.h"
#include <QSettings>

MAVLinkSettingsWidget::MAVLinkSettingsWidget(MAVLinkProtocol* protocol, QWidget *parent) :
    QWidget(parent),
    protocol(protocol),
    m_ui(new Ui::MAVLinkSettingsWidget)
{
    m_ui->setupUi(this);

    m_ui->gridLayout->setAlignment(Qt::AlignTop);

    // AUTH
    m_ui->droneOSCheckBox->setChecked(protocol->getAuthEnabled());
    QSettings settings;
    m_ui->droneOSComboBox->setCurrentIndex(m_ui->droneOSComboBox->findText(settings.value("DRONELINK_HOST", "dronelink.io:14555").toString()));
    m_ui->droneOSLineEdit->setText(protocol->getAuthKey());

    // Initialize state
    m_ui->heartbeatCheckBox->setChecked(protocol->heartbeatsEnabled());
    m_ui->versionCheckBox->setChecked(protocol->versionCheckEnabled());
    m_ui->multiplexingCheckBox->setChecked(protocol->multiplexingEnabled());
    m_ui->systemIdSpinBox->setValue(protocol->getSystemId());

    m_ui->paramGuardCheckBox->setChecked(protocol->paramGuardEnabled());
    m_ui->paramRetransmissionSpinBox->setValue(protocol->getParamRetransmissionTimeout());
    m_ui->paramRewriteSpinBox->setValue(protocol->getParamRewriteTimeout());

    m_ui->actionGuardCheckBox->setChecked(protocol->actionGuardEnabled());
    m_ui->actionRetransmissionSpinBox->setValue(protocol->getActionRetransmissionTimeout());

    // Connect actions
    // Heartbeat
    connect(protocol, SIGNAL(heartbeatChanged(bool)), m_ui->heartbeatCheckBox, SLOT(setChecked(bool)));
    connect(m_ui->heartbeatCheckBox, SIGNAL(toggled(bool)), protocol, SLOT(enableHeartbeats(bool)));
    // Version check
    connect(protocol, SIGNAL(versionCheckChanged(bool)), m_ui->versionCheckBox, SLOT(setChecked(bool)));
    connect(m_ui->versionCheckBox, SIGNAL(toggled(bool)), protocol, SLOT(enableVersionCheck(bool)));
    // System ID
    connect(protocol, SIGNAL(systemIdChanged(int)), m_ui->systemIdSpinBox, SLOT(setValue(int)));
    connect(m_ui->systemIdSpinBox, SIGNAL(valueChanged(int)), protocol, SLOT(setSystemId(int)));
    // Multiplexing
    connect(protocol, SIGNAL(multiplexingChanged(bool)), m_ui->multiplexingCheckBox, SLOT(setChecked(bool)));
    connect(m_ui->multiplexingCheckBox, SIGNAL(toggled(bool)), protocol, SLOT(enableMultiplexing(bool)));
    // Parameter guard
    connect(protocol, SIGNAL(paramGuardChanged(bool)), m_ui->paramGuardCheckBox, SLOT(setChecked(bool)));
    connect(m_ui->paramGuardCheckBox, SIGNAL(toggled(bool)), protocol, SLOT(enableParamGuard(bool)));
    connect(protocol, SIGNAL(paramRetransmissionTimeoutChanged(int)), m_ui->paramRetransmissionSpinBox, SLOT(setValue(int)));
    connect(m_ui->paramRetransmissionSpinBox, SIGNAL(valueChanged(int)), protocol, SLOT(setParamRetransmissionTimeout(int)));
    connect(protocol, SIGNAL(paramRewriteTimeoutChanged(int)), m_ui->paramRewriteSpinBox, SLOT(setValue(int)));
    connect(m_ui->paramRewriteSpinBox, SIGNAL(valueChanged(int)), protocol, SLOT(setParamRewriteTimeout(int)));
    // Action guard
    connect(protocol, SIGNAL(actionGuardChanged(bool)), m_ui->actionGuardCheckBox, SLOT(setChecked(bool)));
    connect(m_ui->actionGuardCheckBox, SIGNAL(toggled(bool)), protocol, SLOT(enableActionGuard(bool)));
    connect(protocol, SIGNAL(actionRetransmissionTimeoutChanged(int)), m_ui->actionRetransmissionSpinBox, SLOT(setValue(int)));
    connect(m_ui->actionRetransmissionSpinBox, SIGNAL(valueChanged(int)), protocol, SLOT(setActionRetransmissionTimeout(int)));
    // MAVLink AUTH
    connect(protocol, SIGNAL(authChanged(bool)), m_ui->droneOSCheckBox, SLOT(setChecked(bool)));
    connect(m_ui->droneOSCheckBox, SIGNAL(toggled(bool)), this, SLOT(enableDroneOS(bool)));
    connect(protocol, SIGNAL(authKeyChanged(QString)), m_ui->droneOSLineEdit, SLOT(setText(QString)));
    connect(m_ui->droneOSLineEdit, SIGNAL(textChanged(QString)), this, SLOT(setDroneOSKey(QString)));

    // Drone OS
    connect(m_ui->droneOSComboBox, SIGNAL(currentIndexChanged(QString)), this, SLOT(setDroneOSHost(QString)));
    // FIXME Manually trigger this action here, this brings control code to UI = BAD!
    setDroneOSHost(m_ui->droneOSComboBox->currentText());

    // Update values
    m_ui->versionLabel->setText(tr("MAVLINK_VERSION: %1").arg(protocol->getVersion()));

    // Connect visibility updates
    connect(protocol, SIGNAL(versionCheckChanged(bool)), m_ui->versionLabel, SLOT(setVisible(bool)));
    m_ui->versionLabel->setVisible(protocol->versionCheckEnabled());
//    // Multiplexing visibility
//    connect(protocol, SIGNAL(multiplexingChanged(bool)), m_ui->multiplexingFilterCheckBox, SLOT(setVisible(bool)));
//    m_ui->multiplexingFilterCheckBox->setVisible(protocol->multiplexingEnabled());
//    connect(protocol, SIGNAL(multiplexingChanged(bool)), m_ui->multiplexingFilterLineEdit, SLOT(setVisible(bool)));
//    m_ui->multiplexingFilterLineEdit->setVisible(protocol->multiplexingEnabled());
    // Param guard visibility
    connect(protocol, SIGNAL(paramGuardChanged(bool)), m_ui->paramRetransmissionSpinBox, SLOT(setVisible(bool)));
    m_ui->paramRetransmissionSpinBox->setVisible(protocol->paramGuardEnabled());
    connect(protocol, SIGNAL(paramGuardChanged(bool)), m_ui->paramRetransmissionLabel, SLOT(setVisible(bool)));
    m_ui->paramRetransmissionLabel->setVisible(protocol->paramGuardEnabled());
    connect(protocol, SIGNAL(paramGuardChanged(bool)), m_ui->paramRewriteSpinBox, SLOT(setVisible(bool)));
    m_ui->paramRewriteSpinBox->setVisible(protocol->paramGuardEnabled());
    connect(protocol, SIGNAL(paramGuardChanged(bool)), m_ui->paramRewriteLabel, SLOT(setVisible(bool)));
    m_ui->paramRewriteLabel->setVisible(protocol->paramGuardEnabled());
    // Action guard visibility
    connect(protocol, SIGNAL(actionGuardChanged(bool)), m_ui->actionRetransmissionSpinBox, SLOT(setVisible(bool)));
    m_ui->actionRetransmissionSpinBox->setVisible(protocol->actionGuardEnabled());
    connect(protocol, SIGNAL(actionGuardChanged(bool)), m_ui->actionRetransmissionLabel, SLOT(setVisible(bool)));
    m_ui->actionRetransmissionLabel->setVisible(protocol->actionGuardEnabled());

    // TODO implement filtering
    // and then remove these two lines
    m_ui->multiplexingFilterCheckBox->setVisible(false);
    m_ui->multiplexingFilterLineEdit->setVisible(false);
}

void MAVLinkSettingsWidget::enableDroneOS(bool enable)
{
    // Enable multiplexing
    protocol->enableMultiplexing(enable);
    // Get current selected host and port
    QString hostString = m_ui->droneOSComboBox->currentText();
    //QString host = hostString.split(":").first();

    LinkManager*    linkMgr = qgcApp()->toolbox()->linkManager();
    UDPLink*        firstUdp = NULL;

    // Delete from all lists first
    for (int i=0; i<linkMgr->links()->count(); i++) {
        LinkInterface*  link = linkMgr->links()->value<LinkInterface*>(i);
        UDPLink*        udp = qobject_cast<UDPLink*>(link);

        if (udp)
        {
            if (!firstUdp) firstUdp = udp;
            // Remove current hosts
            for (int i = 0; i < m_ui->droneOSComboBox->count(); ++i)
            {
                QString oldHostString = m_ui->droneOSComboBox->itemText(i);
                oldHostString = hostString.split(":").first();
                udp->removeHost(oldHostString);
            }
        }
    }

    // Re-add if enabled
    if (enable)
    {
        if (firstUdp)
        {
            firstUdp->addHost(hostString);
        }
        // Set key
        protocol->setAuthKey(m_ui->droneOSLineEdit->text().trimmed());
        QSettings settings;
        settings.setValue("DRONELINK_HOST", m_ui->droneOSComboBox->currentText());
    }
    protocol->enableAuth(enable);
}

void MAVLinkSettingsWidget::setDroneOSKey(QString key)
{
    Q_UNUSED(key);
    enableDroneOS(m_ui->droneOSCheckBox->isChecked());
}

void MAVLinkSettingsWidget::setDroneOSHost(QString host)
{
    Q_UNUSED(host);
    enableDroneOS(m_ui->droneOSCheckBox->isChecked());
}

MAVLinkSettingsWidget::~MAVLinkSettingsWidget()
{
    delete m_ui;
}

void MAVLinkSettingsWidget::changeEvent(QEvent *e)
{
    QWidget::changeEvent(e);
    switch (e->type()) {
    case QEvent::LanguageChange:
        m_ui->retranslateUi(this);
        break;
    default:
        break;
    }
}

void MAVLinkSettingsWidget::hideEvent(QHideEvent* event)
{
    Q_UNUSED(event);
    protocol->storeSettings();
}
