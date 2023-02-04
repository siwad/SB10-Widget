# SB10-Widget (Application)

An application which visualizes the enery flow between solar panels, SonnenBatterie 10, mains and the flat based on a REST API of SonnenBatterie 10.

Additionally this application serves as an example of "How-to connect C++/Qt signals to QML slots ?" and "How-to connect QML signals to C++/Qt slots ?".

## Screenshots of the product

## The REST API of SonnenBatterie 10

## How-to connect C++/Qt signals to QML slots
There are several signals of C++ code connected to QML. The type of the arguments should be QVariant.

#### main.h:
<pre>
class QMLBackend {
   // ...
signals:
	void notifyMainsVoltageFrequencyChanged(QVariant,QVariant);
	void notifyPowerPanelsChanged(QVariant);
};
</pre>
#### main.cpp:
<pre>
void QMLBackend::init(CMainController * pOwner, QQuickItem * pQMLRootItem) {
    // ...

		// pQMLRootItem : QQuickRootItem
		//		child[0]: QQmlConnections  ! AT THIS MOMENT !
		connect(this, SIGNAL(notifyMainsVoltageFrequencyChanged(QVariant,QVariant)), 
                        m_pQMLRootItem->children().first(), 
                              SLOT(notifyPowersupplyVoltageFrequencyChanged(QVariant,QVariant)), Qt::QueuedConnection);
		connect(this, SIGNAL(notifyPowerPanelsChanged(QVariant)), 
                        m_pQMLRootItem->children().first(), 
                              SLOT(notifyPowermeterPanelsChanged(QVariant)), Qt::QueuedConnection);
    
    // ...
}
// ----------------------------------------------------
void CQMLBackend::powerValuesChanged() {
	emit notifyPowerPanelsChanged(m_pOwner->getValue(CMainController::EVT_PowerPanels).c_str());
	emit notifyMainsVoltageFrequencyChanged(m_pOwner->getValue(CMainController::EVT_MainsVoltage).c_str(),
					        m_pOwner->getValue(CMainController::EVT_MainsFrequency).c_str());
  // ...
}
</pre>
#### main.qml:
<pre>
Window {
    // ...

    // The connections between c++ signal and QML slot
    Connections {
        target:  QmlBackend

        // Connections: C++ signals -> QML slots
        function notifyPowermeterPanelsChanged(value) {
            //console.log("Powermeter panels changed")
            powermeter_panels.text = value + " kW"
            }
        function notifyPowersupplyVoltageFrequencyChanged(voltage, frequency) {
            //console.log("Powermeter panels changed")
            powersupply_meter.text = voltage +" V / " + frequency + " Hz"
            }
	// ... and more functions
    } // End of Connections
} // End of Window
</pre>

## How-to connect QML signals to C++/Qt slots
There is one signal of QML connected with one slot of C++ code.

#### main.qml
<pre>
Window {
    // ...
    Column {
        // ...
	
        // Connection: QML signal -> C++ slot
        signal signalEditingFinished(sn: variant, ip: variant, auth: variant, cap: variant)
        // Invoking the QML signal. 
	// Rem.: the function 'editingFinished' is called from TextInput.onEditingFinished
        function editingFinished() {
            signalEditingFinished(edit_serial_number.text, edit_ip_addr.text, 
	                          edit_auth_token.text, edit_capacity.text)
        }
    } // End of main_column_layout
}
</pre>

#### main.h
<pre>
class QMLBackend {
    // ...
protected slots:
    void editingFinished(QVariant,QVariant,QVariant,QVariant);
};
</pre>

#### main.cpp
<pre>
void CQMLBackend::editingFinished(QVariant a, QVariant b, QVariant c, QVariant d) {
    unsigned long serialNumber = a.toULongLong();
    std::string   ipAddress    = b.toString().toStdString();
    std::string   authToken    = c.toString().toStdString();
    unsigned int  capacity     = d.toUInt();
    // ...
}
</pre>

## Building the product
