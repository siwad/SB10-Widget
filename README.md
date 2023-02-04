# SB10-Widget (Application)

An application which visualizes the enery flow between solar panels, SonnenBatterie 10, mains and the flat based on a REST API of SonnenBatterie 10.

Additionally this application serves as an example of "How-to connect C++/Qt signals to QML slots ?" and "How-to connect QML signals to C++/Qt slots ?".

## Screenshots of the product

## The REST API of SonnenBatterie 10

## How-to connect C++/Qt signals to QML slots
Thre are several signals of C++ code connected to QML. The type of the arguments should be QVariant.
main.h:
<pre>
class QMLBackend {
  // ...
signals:
	void notifyMainsVoltageFrequencyChanged(QVariant,QVariant);
	void notifyPowerPanelsChanged(QVariant);
};
</pre>
main.cpp:
<pre>
void QMLBackend::init(CMainController * pOwner, QQuickItem * pQMLRootItem) {
    // ...

		// pQMLRootItem : QQuickRootItem
		//		child[0]: QQmlConnections  ! AT THIS MOMENT !
		connect(this, SIGNAL(notifyMainsVoltageFrequencyChanged(QVariant,QVariant)), m_pQMLRootItem->children().first(), SLOT(notifyPowersupplyVoltageFrequencyChanged(QVariant,QVariant)), Qt::QueuedConnection);
		connect(this, SIGNAL(notifyPowerPanelsChanged(QVariant)), m_pQMLRootItem->children().first(), SLOT(notifyPowermeterPanelsChanged(QVariant)), Qt::QueuedConnection);
    
    // ...
}

void CQMLBackend::powerValuesChanged() {
	emit notifyPowerPanelsChanged(m_pOwner->getValue(CMainController::EVT_PowerPanels).c_str());
	emit notifyMainsVoltageFrequencyChanged(m_pOwner->getValue(CMainController::EVT_MainsVoltage).c_str(),
											m_pOwner->getValue(CMainController::EVT_MainsFrequency).c_str());
  // ...
}
</pre>
main.qml:

## How-to connect QML signals to C++/Qt slots

## Building the product
