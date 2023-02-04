#ifndef DE_BSWALZ_SB10WIDGET_MAIN_H
#define DE_BSWALZ_SB10WIDGET_MAIN_H

/**
 * SonnenBatterie10-Widget: main classes
 *
 * @copyright	2023 Siegfried Walz
 * @license     http://www.gnu.org/licenses/gpl.txt GNU Public License
 * @author      Siegfried Walz <qt-software@bswalz.de>
 * @link        https://software.bswalz.de/
 * @package     SB10-Widget
 */
/*
 * This file is part of SonnenBatterie10-Widget
 *
 * SB10-Widget is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * any later version.
 *
 * SB10-Widget is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with SB10-Widget. If not, see <http://www.gnu.org/licenses/>.
 *
 * SB10-Widget was tested under Android 5 (SDK 21) and Linux and
 * was developped using Qt 5.15.2 .
 */

#include <QObject>
#include <qqml.h> // QML_ELEMENT
#include <memory>
#include <queue>
#include <common/mvc/View.h> // Separate git-repo
#include <common/model/Parameter.h>

class QRunnable;
class QQuickWidget;
class QQuickItem;
class QCoreApplication;
class QSettings;
class QQmlApplicationEngine;
class QNetworkAccessManager;
class QNetworkReply;

namespace de { namespace bswalz {
namespace mvc {
class Model;
}
namespace model {
class CEnumParameter;
}

namespace sb10widget {

using de::bswalz::mvc::View;
using de::bswalz::mvc::Model;
using namespace de::bswalz::model;

class CMainController;

// -----------------------------------------------------------------------
// Class CQMLBackend
// -----------------------------------------------------------------------
class CQMLBackend : public QObject {
	Q_OBJECT
#if QT_VERSION >= QT_VERSION_CHECK(5, 15, 0)
	QML_ELEMENT
#endif

public:
	CQMLBackend(QObject * pParent = nullptr);
	virtual ~CQMLBackend();
	void init(CMainController *, QQuickItem*);
	void tearDown();

	void wifiStatusChanged(const QVariant &);
	void settingsChanged(const QVariant &, const QVariant &, const QVariant &, const QVariant &);
	void configLabelChanged(const QVariant &);
	void updateVersionLabel(const QVariant &);
	void powerValuesChanged();

	void connectQmlSignals();

signals:
	void notifyWifiStatusChanged(QVariant);
	void notifySettingsChanged(QVariant, QVariant, QVariant, QVariant);
	void notifyConfigLabelChanged(QVariant);
	void notifyVersionLabelChanged(const QVariant &);

	void notifyMainsVoltageFrequencyChanged(QVariant,QVariant);
	void notifySB10EnergyChanged(QVariant,QVariant);
	void notifyPowerPanelsChanged(QVariant);
	void notifyPowerHouseChanged(QVariant);
	void notifyPowerPanels2SB10Changed(QVariant);
	void notifyPowerPanels2MainsChanged(QVariant);
	void notifyPowerPanels2HouseChanged(QVariant);
	void notifyPowerMains2HouseChanged(QVariant);
	void notifyPowerSB102HouseChanged(QVariant);

protected slots:
	void editingFinished(QVariant,QVariant,QVariant,QVariant);

private:
	QQuickItem *		m_pQMLRootItem;
	CMainController *	m_pOwner;
};

// -----------------------------------------------------------------------
// Class CMainController (Singleton)
// -----------------------------------------------------------------------
class CMainController : public QObject, public View {
	Q_OBJECT
public:
	enum EWifiStatus  { EWifiNotConnected = 0, EWifiConnected = 1, EWifiSB10Connected = 2 };

	static	CMainController * getInstance();
	virtual ~CMainController();
	void	init(QCoreApplication *, QQmlApplicationEngine &);

	/** Inherited from View */
	virtual void update(const Model * pModel, void * pObject) override;

	/** Access to QML backend */
	CQMLBackend * getQMLBackend() { return &m_QMLBackend; }

	/** Access to Wifi status */
	CEnumParameter * getWifiStatus() const { return m_upWifiStatus.get(); }

	/** @return ip address */
	const std::string & getIpAddr() const { return m_upIpAddress->getValue(); }
	/** @return serial number */
	unsigned long getSerialNumber() const { return m_upSerialNumber->getValue(); }
	/** @return auth-token */
	const std::string & getAuthToken() const { return m_upAuthToken->getValue(); }
	/** Sets the ip address */
	void setIpAddr(const std::string & v) { m_upIpAddress->assignValue(v); }
	/** Sets the serial number */
	void setSerialNumber(unsigned long v) { m_upSerialNumber->assignValue(v); }
	/** Sets the auth-token */
	void setAuthToken(const std::string & v) { m_upAuthToken->assignValue(v); }
	/** Sets the batteries capacity */
	void setBatteryCapacity(unsigned int v) { m_upCapacity->assignValue(v); }

	/** Requests a JSON file with power values of SonnenBatterie10 */
	void    requestJSONFile();

	enum EValueType { EVT_PowerPanels, EVT_MainsVoltage, EVT_MainsFrequency, EVT_PowerHouse, EVT_EnergySB10, EVT_EnergySB10Percentage,
					  EVT_PowerPanels2Mains, EVT_PowerPanels2_SB10, EVT_PowerMains2House, EVT_PowerSB102House,
					  EVT_PowerPanels2House };
	std::string getValue(EValueType);
	void	saveSettings();
protected:
	CMainController();
	void	updateWifiStatus();
	void    updateSettings();
	void    updatePowerValues();
	void    updateConfigLabel();
	void	updateVersionLabel();

protected slots:
	void	tearDown();
	void	httpFinished();
	void	httpReadyRead();
	void    networkError();
	void    sslError();
	void    _requestJSONFile();
#ifdef ANDROID
	void	appStateChanged(Qt::ApplicationState);
#endif

signals:
	void    enqueueRequestJSONFile();

private:
	static	std::unique_ptr<CMainController> m_upInstance;
	QRunnable * m_pNetworkObserver;
	QNetworkAccessManager * m_pNetworkAccessManager;
	QNetworkReply * m_pNetworkReply;
	QSettings *  m_pSettings;
	CQMLBackend m_QMLBackend;

	/** Setting: "IPAddress" */
	std::unique_ptr<CAStringParameter> m_upIpAddress;
	/** Setting: "SerialNumber" */
	std::unique_ptr<CULongParameter> m_upSerialNumber;
	/** Setting: "AuthToken" */
	std::unique_ptr<CAStringParameter> m_upAuthToken;
	/** Setting: "Capacity" */
	std::unique_ptr<CUShortParameter> m_upCapacity;

	std::unique_ptr<CEnumParameter> m_upWifiStatus;
	std::unique_ptr<CFloatParameter>  m_upMainsFrequency;
	std::unique_ptr<CFloatParameter>  m_upMainsVoltage;
	std::unique_ptr<CIntParameter>  m_upSB10Energy;
	std::unique_ptr<CIntParameter>  m_upSB10EnergyPercentage;
	std::unique_ptr<CIntParameter>  m_upPowerPanels;
	std::unique_ptr<CIntParameter>  m_upPowerHouse;
	std::unique_ptr<CIntParameter>  m_upPowerPanels2SB10;
	std::unique_ptr<CIntParameter>  m_upPowerPanels2Mains;
	std::unique_ptr<CIntParameter>  m_upPowerPanels2House;
	std::unique_ptr<CIntParameter>  m_upPowerMains2House;
	std::unique_ptr<CIntParameter>  m_upPowerSB102House;
	std::unique_ptr<CAStringParameter>  m_upTimestamp;
};


}}} // End namespaces

#endif // DE_BSWALZ_SB10WIDGET_MAIN_H
