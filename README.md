# SB10-Widget (Application)

An application which visualizes the enery flow between solar panels, SonnenBatterie 10, mains and the flat based on a REST API of SonnenBatterie 10.

Additionally this application serves as an example of "How-to connect C++/Qt signals to QML slots ?" and "How-to connect QML signals to C++/Qt slots ?".

<strong>Important Note</strong>: 
SB10-Widget is distributed in the hope that it will be useful, but WITHOUT ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE; without even any warranty of MISBEHAVIOUR,DAMAGE of SonnenBatterie 10 due to faults,misuse of REST API.

## Screenshots of the product
![Screenshot_20230209_154130](https://user-images.githubusercontent.com/107047007/217846446-3c0e9811-7e8d-4f19-8407-2b2fb67b80bf.png)

## The REST API of SonnenBatterie 10
A query to the status of SonnenBatterie like this
<pre>
curl http://192.168.1.36:80/api/v2/status > ./status.json
</pre>
gets a JSON file as result. Remind:
* no 'Auth-Token' is required in the HTTP header for 'status'
* the IP address 192.168.1.36 is just an example of the network address of SonnenBatterie 10.
The resulting JSON file has the following structure (shortened):
<pre>
{
"Consumption_W":1853,
"Fac":49.97800064086914,
"GridFeedIn_W":-33,
"Pac_total_W":1123,
"Production_W":702,
"RSOC":52,
"Timestamp":"2023-01-17 12:51:05",
"USOC":47,
"Uac":233,
}
</pre>
SB10-Widget uses the key/value pairs of 
* "Production_W": PV production in Watts [integer]
* "Uac": AC voltage in Volts [integer]
* "Fac": AC frequency in Hertz [float]
* "USOC": User state of charge in percent [integer]
* "Consumption_W": consumption of the building in Watts, direct measurement [integer]
* "GridFeedIn_W": Grid Feed is negative at consumption and positive if it feeds. Unit is Watts [integer]
* "Pac_total_W": AC power greater than ZERO is discharging, AC Power less than ZERO is charging. Unit in Watts [integer]

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
void CQMLBackend::connectQmlSignals() {
	connect(m_pQMLRootItem, SIGNAL(signalEditingFinished(QVariant,QVariant,QVariant,QVariant)),
		this, SLOT(editingFinished(QVariant,QVariant,QVariant,QVariant)), Qt::QueuedConnection);
}
// ----------------------------------------------------
void CQMLBackend::editingFinished(QVariant a, QVariant b, QVariant c, QVariant d) {
    unsigned long serialNumber = a.toULongLong();
    std::string   ipAddress    = b.toString().toStdString();
    std::string   authToken    = c.toString().toStdString();
    unsigned int  capacity     = d.toUInt();
    // ...
}
</pre>

## Building the product
Checkout of the sources
<pre>
git clone https://github.com/siwad/SB10-Widget
cd SB10-Widget
git clone https://github.com/siwad/common
</pre>

I built and tested the product with the following toolchain resp. kits:
* QtCreator 5.9
* CMake 3.17.0
* Desktop_Qt-5.15.2_GCC_64_bit (running on [openSuSE](https://www.opensuse.org) Linux)
* Android_Qt-5.15.2_Clang_Multi_Abi (running on Android:tm: emulation of [Sailfish OS](https://sailfishos.org/)), API Level 21
* gcc 7.5.0
