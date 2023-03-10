
/**
 * SB10-Widget: main function / main classes
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

#include "main.h"
#include <common/network/networkhelper.h> // Separate git-repo
#include <common/model/Parameter.h>       // Separate git-repo
#include <common/model/EnumParameter.h>   // Separate git-repo
#include <QGuiApplication>
#include <QQmlApplicationEngine>
#include <QQmlContext>
#include <QtQuickWidgets/QQuickWidget>
#include <QQuickItem>

#include <QLocale>
#include <QTranslator>
#include <QRunnable>
#include <QThreadPool>
#include <QSettings>
#include <QNetworkInterface>
#include <QNetworkConfigurationManager>
#include <QNetworkAccessManager>
#include <QNetworkRequest>
#include <QNetworkReply>
#include <QGuiApplication>
#include <QQmlApplicationEngine>
#include <QJsonDocument>
#include <QJsonObject>
#include <math.h>


int main(int argc, char *argv[]) {
#if QT_VERSION < QT_VERSION_CHECK(6, 0, 0)
	QCoreApplication::setAttribute(Qt::AA_EnableHighDpiScaling);
#endif
	QGuiApplication app(argc, argv);
	app.setOrganizationName("sb10widget");
	app.setOrganizationDomain("bswalz.de");
	app.setApplicationName("SB10-Widget");

	QTranslator translator;
	const QStringList uiLanguages = QLocale::system().uiLanguages();
	for (const QString &locale : uiLanguages) {
		const QString baseName = "OlyCamera-RC_" + QLocale(locale).name();
		if (translator.load(":/i18n/" + baseName)) {
			app.installTranslator(&translator);
			break;
		}
	}

#if QT_VERSION < QT_VERSION_CHECK(5, 15, 0)
	// Let you import it with "import ModuleName 1.0" on the QML side. Replacement for QML_ELEMENT
	qmlRegisterType<de::bswalz::olycamerarc::CQMLBackend>("de.bswalz.olycamerarc", 1, 0, "QmlBackend");
#endif

	QQmlApplicationEngine engine;
	de::bswalz::sb10widget::CMainController::getInstance(); // Instantiates main controller

	// Exposing C++ object to Qml
	de::bswalz::sb10widget::CQMLBackend * pQMLBackend = de::bswalz::sb10widget::CMainController::getInstance()->getQMLBackend();
	engine.rootContext()->setContextProperty("QmlBackend", pQMLBackend);

	const QUrl url(QStringLiteral("qrc:/main.qml"));
	QObject::connect(&engine, &QQmlApplicationEngine::objectCreated,
					 &app, [url,&app,&engine](QObject *obj, const QUrl &objUrl) {
									if (obj == nullptr && url == objUrl)
										QCoreApplication::exit(-1);
									else if (url == objUrl) {
										// Creates an initializes the application and the main controller
										app.setWindowIcon(QIcon(":/res/icon.png"));
										de::bswalz::sb10widget::CMainController::getInstance()->init(&app, engine);
										}
					}, Qt::QueuedConnection);
	engine.load(url);

	return app.exec();
}



namespace de { namespace bswalz { namespace sb10widget {
namespace {
// -----------------------------------------------------------------------
// Anonymous class CNetworkObserver
// -----------------------------------------------------------------------
class CNetworkObserver : public QRunnable {
public:
	CNetworkObserver(CMainController * pMainController)
			: QRunnable(), m_pOwner(pMainController), m_StopRequest(false) {}
	virtual ~CNetworkObserver() {}
	virtual void run() override;
	void    stopRequest() { m_StopRequest = true; }
private:
	CMainController * m_pOwner;
	bool              m_StopRequest;
};

} // End anonymous namespace


// -----------------------------------------------------------------------
// Implementation of class CMainController
// -----------------------------------------------------------------------
std::unique_ptr<CMainController> CMainController::m_upInstance = std::unique_ptr<CMainController>();

// -----------------------------------------------------------------------
CMainController * CMainController::getInstance() {
	if (m_upInstance.get() == nullptr)
		m_upInstance.reset(new CMainController());
	return m_upInstance.get();
}

// -----------------------------------------------------------------------
void CMainController::init(QCoreApplication * pApp, QQmlApplicationEngine & engine) {
	var_array<int> wifi_values = { (int)EWifiNotConnected, (int)EWifiConnected, (int)EWifiSB10Connected };
	m_upWifiStatus.reset( new CEnumParameter("WifiStatus", EWifiNotConnected, wifi_values) );

	m_upSerialNumber.reset( new CULongParameter("SerialNumber", 0, 0, 0xFFFFFFFFuL) );
	m_upIpAddress.reset( new CAStringParameter("IPAddress", "192.168.1.1") );
	m_upAuthToken.reset( new CAStringParameter("AuthToken", "########-####-####-####-############") );
	m_upCapacity.reset( new CUShortParameter("Capacity", 5, 5, 20) );

	m_upTimestamp.reset( new CAStringParameter("Timestamp", "YYYY-MM-DD") );
	m_upMainsFrequency.reset( new CFloatParameter("MainsFrequency", 50.0f, 40.0f, 60.0f) );
	m_upMainsVoltage.reset( new CFloatParameter("MainsVoltage", 230.0f, 190.0f, 270.0f) );;
	m_upSB10Energy.reset( new CIntParameter("SB10Energy", 0, 0, INT_MAX) );
	m_upSB10EnergyPercentage.reset( new CIntParameter("SB10EnergyPercentage", 0, 0, 100) );
	m_upPowerPanels.reset( new CIntParameter("PowerPanels", 0, 0, INT_MAX) );
	m_upPowerHouse.reset( new CIntParameter("PowerHouse", 0, 0, INT_MAX) );
	m_upPowerPanels2SB10.reset( new CIntParameter("PowerPanels2SB10", 0, 0, INT_MAX) );
	m_upPowerPanels2Mains.reset( new CIntParameter("PowerPanels2Mains", 0, 0, INT_MAX) );
	m_upPowerPanels2House.reset( new CIntParameter("PowerPanels2House", 0, 0, INT_MAX) );
	m_upPowerMains2House.reset( new CIntParameter("PowerMains2House", 0, 0, INT_MAX) );
	m_upPowerSB102House.reset( new CIntParameter("PowerSB102House", 0, 0, INT_MAX) );

	m_pNetworkAccessManager = new QNetworkAccessManager();

	// Manages the settings
	m_pSettings = new QSettings("de.bswalz/sb10-widget");
	QString  fn = m_pSettings->fileName();
	m_pSettings->sync();
	QVariant value = m_pSettings->value( QString::fromStdString(m_upAuthToken->getName()) );
	if (!value.isNull() && value.isValid())
		m_upAuthToken->assignValue(value.toString().toStdString());
	value = m_pSettings->value( QString::fromStdString(m_upIpAddress->getName()) );
	if (!value.isNull() && value.isValid())
		m_upIpAddress->assignValue(value.toString().toStdString());
	value = m_pSettings->value( QString::fromStdString(m_upSerialNumber->getName()) );
	if (!value.isNull() && value.isValid())
		m_upSerialNumber->assignValue((unsigned long)value.toULongLong());
	value = m_pSettings->value( QString::fromStdString(m_upCapacity->getName()) );
	if (!value.isNull() && value.isValid())
		m_upCapacity->assignValue(value.toUInt());

	// The structure must mirror the QML structure (see main.qml)
	QQuickWidget * pRootWidget = static_cast<QQuickWidget*>(engine.rootObjects().first());
	for (auto child : pRootWidget->children()) {
		QQuickItem * pItem = dynamic_cast<QQuickItem*>(child);
		if (pItem != nullptr)
			m_QMLBackend.init(this, pItem);
		}

	connect(pApp, SIGNAL(aboutToQuit()), this, SLOT(tearDown()));
	connect(this, SIGNAL(enqueueRequestJSONFile()), this, SLOT(_requestJSONFile()), Qt::QueuedConnection);
#ifdef ANDROID
	connect(pApp, SIGNAL(applicationStateChanged(Qt::ApplicationState)), this, SLOT(appStateChanged(Qt::ApplicationState)));
#endif

	registerAt(m_upWifiStatus.get(), true /*Notifies QML widget*/);
	registerAt(m_upSerialNumber.get(), true /*Notifies QML widget*/);
	registerAt(m_upTimestamp.get(), false /*Does not notify QML widget*/);

	m_QMLBackend.connectQmlSignals();

	QThreadPool::globalInstance()->start(m_pNetworkObserver);
}

// -----------------------------------------------------------------------
void CMainController::tearDown() {
	disconnect(this, SIGNAL(enqueueRequestJSONFile()), this, SLOT(_requestJSONFile()));

	// Saves the settings at ...
	saveSettings();
	delete m_pSettings;
	m_pSettings = nullptr;


	dynamic_cast<CNetworkObserver*>(m_pNetworkObserver)->stopRequest();
	m_QMLBackend.tearDown();
	QThread::msleep(800);

	unregisterAt(m_upWifiStatus.get());
	unregisterAt(m_upSerialNumber.get());
	unregisterAt(m_upTimestamp.get());

	delete m_pNetworkAccessManager;
	m_pNetworkAccessManager = nullptr;
}

#ifdef ANDROID
// -----------------------------------------------------------------------
void CMainController::appStateChanged(Qt::ApplicationState state) {
	if (state != Qt::ApplicationSuspended)
		return;
	// Saves the settings on ANDROID devices
	saveSettings();
}
#endif

// -----------------------------------------------------------------------
void CMainController::saveSettings() {
	if (m_pSettings == nullptr) return;
	m_pSettings->setValue(QString::fromStdString(m_upAuthToken->getName()),
						  QString::fromStdString(m_upAuthToken->getValue()));
	m_pSettings->setValue(QString::fromStdString(m_upIpAddress->getName()),
						  QString::fromStdString(m_upIpAddress->getValue()));
	m_pSettings->setValue(QString::fromStdString(m_upSerialNumber->getName()),
						  QString::number(m_upSerialNumber->getValue()));
	m_pSettings->setValue(QString::fromStdString(m_upCapacity->getName()),
						  QString::number(m_upCapacity->getValue()));
}

// -----------------------------------------------------------------------
// Inherited from View
void CMainController::update(const Model * pModel, void * pObject) {
	if (pModel == m_upWifiStatus.get()) {
		updateWifiStatus();
		updateConfigLabel();
		updateVersionLabel();
		}
	else if (pModel == m_upSerialNumber.get() || pModel == m_upAuthToken.get() ||
			 pModel == m_upIpAddress.get()    || pModel == m_upCapacity.get())
		updateSettings();
	else if (pModel == m_upTimestamp.get())
		updatePowerValues();
}

// -----------------------------------------------------------------------
CMainController::CMainController()
	: QObject(), m_pNetworkObserver(nullptr), m_upWifiStatus(nullptr),
	  m_pNetworkAccessManager(nullptr), m_pNetworkReply(nullptr), m_pSettings(nullptr) {
	CNetworkObserver * pObserver = new CNetworkObserver(this);
	m_pNetworkObserver           = pObserver;
	m_pNetworkObserver->setAutoDelete(true);
}

// -----------------------------------------------------------------------
CMainController::~CMainController() {
	if (m_pNetworkObserver != nullptr && !m_pNetworkObserver->autoDelete()) {
		delete m_pNetworkObserver;
		}
}

// -----------------------------------------------------------------------
void CMainController::updateWifiStatus() {
	EWifiStatus status = (EWifiStatus)m_upWifiStatus->getValue();
	switch (status) {
		case EWifiSB10Connected : qDebug("SonnenBatterie-10 found");	break;
		case EWifiConnected : qDebug("Any Wifi found"); break;
		default: qDebug("No Wifi found"); break;
		}
	m_QMLBackend.wifiStatusChanged(QVariant((int)status));
}

// -----------------------------------------------------------------------
void CMainController::updateSettings() {
	m_QMLBackend.settingsChanged(QVariant(QString("%1").arg((qlonglong)getSerialNumber(), 6, 10, QChar('0'))),
								 QVariant(QString::fromStdString(getIpAddr())),
								 QVariant(QString::fromStdString(getAuthToken())),
								 QVariant(QString::number(m_upCapacity->getValue())));
}

// -----------------------------------------------------------------------
void CMainController::updateConfigLabel() {
	if (m_pSettings != nullptr)
		m_QMLBackend.configLabelChanged(m_pSettings->fileName());
}

// -----------------------------------------------------------------------
void CMainController::updatePowerValues() {
	m_QMLBackend.powerValuesChanged();
}

// -----------------------------------------------------------------------
void CMainController::updateVersionLabel() {
	QDate date = QDate(QLocale("en_US").toDate(QString(__DATE__).simplified(), "MMM d yyyy"));
	m_QMLBackend.updateVersionLabel(date.toString("yyyy-MM-dd"));
}

// -----------------------------------------------------------------------
std::string CMainController::getValue(CMainController::EValueType type) {
	auto power2kWString = [](int power)->std::string {
				std::string result = "-.--";
				if (::abs(power) == 0)
					result = "0.00";
				else if (::abs(power) < 50)
					result = QString::number(power/1000.0f, 'f', 3).toStdString();
				else if (::abs(power) >= 50 && ::abs(power) < 1000)
					result = QString::number(round(power/10.0f)/100.0f, 'f', 2).toStdString(); // 2023-03-10: rounding revised
				else
					result = QString::number(round(power/100.0f)/10.0f, 'f', 1).toStdString(); // 2023-03-10: rounding revised

				return result;
				};

	std::string result = "-.--";
	switch (type) {
		case EVT_PowerPanels :
					result = power2kWString(m_upPowerPanels->getValue());
					break;
		case EVT_MainsVoltage :
					result = QString::number(m_upMainsVoltage->getValue(), 'f', 0).toStdString();
					break;
		case EVT_MainsFrequency :
					result = QString::number((m_upMainsFrequency->getValue()), 'f', 2).toStdString();
					break;
		case EVT_EnergySB10 :
					result = power2kWString(m_upSB10Energy->getValue());
					break;
		case EVT_EnergySB10Percentage :
					result = std::to_string(m_upSB10EnergyPercentage->getValue());
					break;
		case EVT_PowerHouse :
					result = power2kWString(m_upPowerHouse->getValue());
					break;
		case EVT_PowerPanels2_SB10 :
					result = power2kWString(m_upPowerPanels2SB10->getValue());
					break;
		case EVT_PowerSB102House :
					result = power2kWString(m_upPowerSB102House->getValue());
					break;
		case EVT_PowerMains2House :
					result = power2kWString(m_upPowerMains2House->getValue());
					break;
		case EVT_PowerPanels2Mains :
					result = power2kWString(m_upPowerPanels2Mains->getValue());
					break;
		case EVT_PowerPanels2House :
					result = power2kWString(m_upPowerPanels2House->getValue());
					break;
		default:	break;
		}
	return result;
}

// -----------------------------------------------------------------------
void CMainController::requestJSONFile() {
	emit enqueueRequestJSONFile(); // .. because requetsJSONFile ist called by an other thread
}

// -----------------------------------------------------------------------
void CMainController::_requestJSONFile() {
	QUrl url( QString("http://%1:80/api/v2/status").arg(QString::fromStdString(getIpAddr())) );
	QNetworkRequest request(url);
	//request.setRawHeader(QByteArray("Auth-Token"), QByteArray(getAuthToken().c_str()));
	m_pNetworkReply = m_pNetworkAccessManager->get(request);
	//connect(m_pNetworkReply, SIGNAL(finished()), this, SLOT(httpFinished()));
	connect(m_pNetworkReply, &QNetworkReply::finished, this, &de::bswalz::sb10widget::CMainController::httpFinished);
	//connect(((QIODevice*)m_pNetworkReply), SIGNAL(readyRead()), this, SLOT(httpReadyRead()));
	connect(m_pNetworkReply, &QIODevice::readyRead, this, &CMainController::httpReadyRead);
	connect(m_pNetworkReply, &QNetworkReply::errorOccurred, this, &de::bswalz::sb10widget::CMainController::networkError);
	connect(m_pNetworkReply, &QNetworkReply::sslErrors, this, &CMainController::sslError);
}

// -----------------------------------------------------------------------
// Qt slot
void CMainController::httpFinished() {
	if (m_pNetworkReply == nullptr) return;

	if (m_pNetworkReply->error() != QNetworkReply::NoError) {
		m_pNetworkReply->deleteLater();
		m_pNetworkReply = nullptr;
		m_upWifiStatus->assignValue(CMainController::EWifiConnected);
		return;
		}

	QByteArray reply = m_pNetworkReply->readAll();
	m_pNetworkReply->deleteLater();
	m_pNetworkReply = nullptr;

	QJsonParseError parseError;
	QJsonDocument jsonDoc = QJsonDocument::fromJson(reply, &parseError);
	if (jsonDoc.isObject()) {
		QJsonObject object = jsonDoc.object();
		QJsonValue  powerPanels    = object.value("Production_W");
		QJsonValue  mainsVoltage   = object.value("Uac");
		QJsonValue  mainsFrequency = object.value("Fac");
		QJsonValue  energySB10     = object.value("USOC");
		QJsonValue  consumption    = object.value("Consumption_W");
		QJsonValue  gridFeedIn     = object.value("GridFeedIn_W");
		QJsonValue  powerSB10      = object.value("Pac_total_W");
		if (!powerPanels.isNull())
			{ m_upPowerPanels->assignValue(powerPanels.toInt()); }
		if (!mainsVoltage.isNull())
			{ m_upMainsVoltage->assignValue((float)mainsVoltage.toDouble()); }
		if (!mainsFrequency.isNull())
			{ m_upMainsFrequency->assignValue((float)mainsFrequency.toDouble()); }
		if (!energySB10.isNull()) {
			int energy = energySB10.toInt();
			m_upSB10EnergyPercentage->assignValue(energy);
			m_upSB10Energy->assignValue((float)energy * (float)m_upCapacity->getValue() /*[kWh]*/ * 10.0f); // Result: [Wh]
			}
		if (!consumption.isNull())
			{ m_upPowerHouse->assignValue(consumption.toInt()); }
		if (!gridFeedIn.isNull()) {
			int feed_in = gridFeedIn.toInt(); // >0: feed in | <0: power mains -> house
			if (feed_in >= 0)
				m_upPowerPanels2Mains->assignValue(feed_in);
			else
				m_upPowerPanels2Mains->assignValue(0);
			}
		if (!powerSB10.isNull()) {
			int pwr = powerSB10.toInt();
			if (pwr < 0) {
				m_upPowerPanels2SB10->assignValue(-pwr);
				m_upPowerSB102House->assignValue(0);
				}
			else {
				m_upPowerPanels2SB10->assignValue(0);
				m_upPowerSB102House->assignValue(pwr);
				}
			}
		// Calculated from previous values:
		m_upPowerMains2House->assignValue(m_upPowerHouse->getValue() - m_upPowerSB102House->getValue() - m_upPowerPanels2House->getValue());
		m_upPowerPanels2House->assignValue(m_upPowerPanels->getValue() - m_upPowerPanels2Mains->getValue() - m_upPowerPanels2SB10->getValue());

		// Finally
		QJsonValue timestamp = object.value("Timestamp");
		if (timestamp.isString())
			m_upTimestamp->assignValue((timestamp.toString().toStdString()));
		}

	// ToDo: read and interprete the JSON file
}

// -----------------------------------------------------------------------
// Qt slot
void CMainController::httpReadyRead() {
	// This slot gets called every time the QNetworkReply has new data in its buffer.
	// Here is the possibility to read these data

	// Intentionally left blank
}

// -----------------------------------------------------------------------
// Qt slot
void CMainController::networkError() {
	m_upWifiStatus->assignValue(CMainController::EWifiConnected); // At least connection to SonnenBatterie lost.
																  // Tries to re-connect in CNetworkObserver::run()
}

// -----------------------------------------------------------------------
// Qt slot
void CMainController::sslError() { }

// -----------------------------------------------------------------------
// Class CQMLBackend
// -----------------------------------------------------------------------
CQMLBackend::CQMLBackend(QObject * pParent) : QObject(pParent), m_pQMLRootItem(nullptr), m_pOwner(nullptr) {
	// intentionally left blank
}

// -----------------------------------------------------------------------
CQMLBackend::~CQMLBackend() {
	// intentionally left blank
}

// -----------------------------------------------------------------------
void CQMLBackend::init(CMainController * pOwner, QQuickItem * pQMLRootItem) {
	m_pQMLRootItem = pQMLRootItem;
	m_pOwner       = pOwner;

	if (m_pQMLRootItem != nullptr) {	// The structure must match the QML structure (see main.qml). SLOTs at QQuickConnection
		// pQMLRootItem : QQuickRootItem
		//		child[0]: QQmlConnections  ! AT THIS MOMENT !
		connect(this, SIGNAL(notifyWifiStatusChanged(QVariant)), m_pQMLRootItem->children().first(), SLOT(notifyWifiStatusChanged(QVariant)), Qt::QueuedConnection);
		connect(this, SIGNAL(notifySettingsChanged(QVariant,QVariant,QVariant,QVariant)), m_pQMLRootItem->children().first(), SLOT(notifySettingsChanged(QVariant,QVariant,QVariant,QVariant)), Qt::QueuedConnection);
		connect(this, SIGNAL(notifyConfigLabelChanged(QVariant)), m_pQMLRootItem->children().first(), SLOT(notifyConfigLabelChanged(QVariant)), Qt::QueuedConnection);
		connect(this, SIGNAL(notifyVersionLabelChanged(QVariant)), m_pQMLRootItem->children().first(), SLOT(notifyVersionLabelChanged(QVariant)), Qt::QueuedConnection);

		connect(this, SIGNAL(notifyMainsVoltageFrequencyChanged(QVariant,QVariant)), m_pQMLRootItem->children().first(), SLOT(notifyPowersupplyVoltageFrequencyChanged(QVariant,QVariant)), Qt::QueuedConnection);
		connect(this, SIGNAL(notifySB10EnergyChanged(QVariant,QVariant)), m_pQMLRootItem->children().first(), SLOT(notifyEnergymeterSonnenBatterieChanged(QVariant,QVariant)), Qt::QueuedConnection);
		connect(this, SIGNAL(notifyPowerPanelsChanged(QVariant)), m_pQMLRootItem->children().first(), SLOT(notifyPowermeterPanelsChanged(QVariant)), Qt::QueuedConnection);
		connect(this, SIGNAL(notifyPowerHouseChanged(QVariant)), m_pQMLRootItem->children().first(), SLOT(notifyPowermeterPowerConsumptionChanged(QVariant)), Qt::QueuedConnection);
		connect(this, SIGNAL(notifyPowerPanels2SB10Changed(QVariant)), m_pQMLRootItem->children().first(), SLOT(notifyPowermeterPanels2SonnenBatterieChanged(QVariant)), Qt::QueuedConnection);
		connect(this, SIGNAL(notifyPowerPanels2MainsChanged(QVariant)), m_pQMLRootItem->children().first(), SLOT(notifyPowermeterPanels2NetzChanged(QVariant)), Qt::QueuedConnection);
		connect(this, SIGNAL(notifyPowerPanels2HouseChanged(QVariant)), m_pQMLRootItem->children().first(), SLOT(notifyPowermeterPanels2HouseChanged(QVariant)), Qt::QueuedConnection);
		connect(this, SIGNAL(notifyPowerMains2HouseChanged(QVariant)), m_pQMLRootItem->children().first(), SLOT(notifyPowermeterNetz2HouseChanged(QVariant)), Qt::QueuedConnection);
		connect(this, SIGNAL(notifyPowerSB102HouseChanged(QVariant)), m_pQMLRootItem->children().first(), SLOT(notifyPowermeterSonnenBatterie2HouseChanged(QVariant)), Qt::QueuedConnection);
		}
}

// -----------------------------------------------------------------------
void CQMLBackend::tearDown() {
	if (m_pQMLRootItem != nullptr)
		; // It seems there is no need to disconnect notifyWifiStatusChanged() and notifyLifeViewImageChanged
}

// -----------------------------------------------------------------------
void CQMLBackend::connectQmlSignals() {
	connect(m_pQMLRootItem, SIGNAL(signalEditingFinished(QVariant,QVariant,QVariant,QVariant)),
			this, SLOT(editingFinished(QVariant,QVariant,QVariant,QVariant)), Qt::QueuedConnection);
}

// -----------------------------------------------------------------------
void CQMLBackend::wifiStatusChanged(const QVariant & status) {
	emit notifyWifiStatusChanged(status);

	if (status != CMainController::EWifiSB10Connected) {
		emit notifyPowerPanelsChanged(QVariant("---.-"));
		emit notifyMainsVoltageFrequencyChanged(QVariant("---"), QVariant("--.--"));
		emit notifySB10EnergyChanged(QVariant("----"), QVariant("--"));
		emit notifyPowerHouseChanged(QVariant("---.-"));
		emit notifyPowerPanels2SB10Changed(QVariant("---.-"));
		emit notifyPowerPanels2MainsChanged(QVariant("---.-"));
		emit notifyPowerPanels2HouseChanged(QVariant("---.-"));
		emit notifyPowerMains2HouseChanged(QVariant("---.-"));
		emit notifyPowerSB102HouseChanged(QVariant("---.-"));
		}
}

// -----------------------------------------------------------------------
void CQMLBackend::settingsChanged(const QVariant & number, const QVariant & ipaddr,
								  const QVariant & authtoken, const QVariant & capacity) {
	emit notifySettingsChanged(number, ipaddr, authtoken, capacity);
}

// -----------------------------------------------------------------------
void CQMLBackend::configLabelChanged(const QVariant & filename) {
	emit notifyConfigLabelChanged(filename);
}

// -----------------------------------------------------------------------
void CQMLBackend::updateVersionLabel(const QVariant & version) {
	emit notifyVersionLabelChanged(version);
}

// -----------------------------------------------------------------------
void CQMLBackend::powerValuesChanged() {
	emit notifyPowerPanelsChanged(m_pOwner->getValue(CMainController::EVT_PowerPanels).c_str());
	emit notifyMainsVoltageFrequencyChanged(m_pOwner->getValue(CMainController::EVT_MainsVoltage).c_str(),
											m_pOwner->getValue(CMainController::EVT_MainsFrequency).c_str());
	emit notifySB10EnergyChanged(m_pOwner->getValue(CMainController::EVT_EnergySB10).c_str(),
								 m_pOwner->getValue(CMainController::EVT_EnergySB10Percentage).c_str());
	emit notifyPowerHouseChanged(m_pOwner->getValue(CMainController::EVT_PowerHouse).c_str());
	emit notifyPowerPanels2SB10Changed(m_pOwner->getValue(CMainController::EVT_PowerPanels2_SB10).c_str());
	emit notifyPowerPanels2MainsChanged(m_pOwner->getValue(CMainController::EVT_PowerPanels2Mains).c_str());
	emit notifyPowerPanels2HouseChanged(m_pOwner->getValue(CMainController::EVT_PowerPanels2House).c_str());
	emit notifyPowerMains2HouseChanged(m_pOwner->getValue(CMainController::EVT_PowerMains2House).c_str());
	emit notifyPowerSB102HouseChanged(m_pOwner->getValue(CMainController::EVT_PowerSB102House).c_str());
}

// -----------------------------------------------------------------------
void CQMLBackend::editingFinished(QVariant a, QVariant b, QVariant c, QVariant d) {
	unsigned long serialNumber = a.toULongLong();
	std::string   ipAddress    = b.toString().toStdString();
	std::string   authToken    = c.toString().toStdString();
	unsigned int  capacity     = d.toUInt();
	m_pOwner->setSerialNumber(serialNumber);
	m_pOwner->setIpAddr(ipAddress);
	m_pOwner->setAuthToken(authToken);
	m_pOwner->setBatteryCapacity(capacity);
#ifdef ANDROID
	m_pOwner->saveSettings();
#endif
}

namespace {
// -----------------------------------------------------------------------
// Anonymous class CNetworkObserver
// -----------------------------------------------------------------------
void CNetworkObserver::run() {
	const unsigned short TIMEOUT_INTERVAL =   200; // ms
	const unsigned short    WIFI_INTERVAL =  2000; // ms
	setAutoDelete(true);
	unsigned short wifiCounter = 0;
	while(!m_StopRequest) {
		QThread::msleep(TIMEOUT_INTERVAL);

		if ((wifiCounter % (WIFI_INTERVAL/TIMEOUT_INTERVAL)) == 0) {
			QList<QNetworkInterface> interfaces = QNetworkInterface::allInterfaces();
			bool                   anyWifiFound = m_pOwner->getWifiStatus()->getValue() != CMainController::EWifiNotConnected;
			bool                      sb10Found = m_pOwner->getWifiStatus()->getValue() == CMainController::EWifiSB10Connected;
			if (!anyWifiFound || !sb10Found) {
				for (auto &interface : interfaces) {
					if (interface.isValid() && (interface.flags() & QNetworkInterface::IsUp) > 0) {
						anyWifiFound          = true;
						std::string ping_args = "ping -c1 -s1 " + m_pOwner->getIpAddr() + " > /dev/null 2>&1";
						if (system(ping_args.c_str()) == 0)
							{ sb10Found = true; break; }
						} // End if interface ist up and valid
					} // End for all interfaces
				} // End if anyWifi NOT found or sb10 NOT found
			if (anyWifiFound && !sb10Found) {
				m_pOwner->getWifiStatus()->assignValue(CMainController::EWifiConnected);
				}
			else if (sb10Found) {
				m_pOwner->getWifiStatus()->assignValue(CMainController::EWifiSB10Connected);
				m_pOwner->requestJSONFile();
				}
			else {
				m_pOwner->getWifiStatus()->assignValue(CMainController::EWifiNotConnected);
				}
			} // End if wifiCounter >= 10
		wifiCounter++;
	} // End while
}

} // End of anonymous namespace

}}} // End namespaces

