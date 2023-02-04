import QtQuick 2.15
import QtQuick.Window 2.15
import QtQuick.Controls 2.15


Window {
    id: main_window
    title: "SonnenBatterie 10"
    width: 480
    height: 640
    minimumWidth: 240
    minimumHeight: 360
    visible: true

    Column {
        spacing: 2
        padding: 2
        id: main_column_layout

        // Title and LEDs
        Row {
            spacing: 2
            x: 10
            Column {
                Text {
                    text: "SonnenBatterie 10"
                    horizontalAlignment: Text.AlignLeft
                    font.pointSize: 16
                    font.weight: Font.bold
                    width: main_window.width-80
                    } // End Text
                Text {
                    id: serial_number
                    text: "<SerienNummer>"
                    horizontalAlignment: Text.AlignLeft
                    font.pointSize: 8
                    } // End Text
                } // End Column layout of title
            Image {
                id: wifi
                height: 30
                width: 30
                antialiasing: true
                fillMode: Image.PreserveAspectFit
                cache: false
                source: "qrc:/res/wifi-disabled.png"
                }
            Image {
                id: wifi_led
                height: 30
                width: 30
                source: "qrc:/res/led-gr.png"
                }
            } // End Row layout

        Row {
            id: grid_layout
            spacing: 2
            x: 10
            topPadding: 30

            // Code to draw a simple arrow on TypeScript canvas got from https://stackoverflow.com/a/64756256/867349
            function arrow(context, fromx, fromy, tox, toy) {
                 const dx = tox - fromx;
                 const dy = toy - fromy;
                 const headlen = Math.sqrt(dx * dx + dy * dy) * 0.3; // length of head in pixels
                 const angle = Math.atan2(dy, dx);
                 context.beginPath();
                 context.moveTo(fromx, fromy);
                 context.lineTo(tox, toy);
                 context.stroke();
                 context.beginPath();
                 context.moveTo(tox - headlen * Math.cos(angle - Math.PI / 6), toy - headlen * Math.sin(angle - Math.PI / 6));
                 context.lineTo(tox, toy );
                 context.lineTo(tox - headlen * Math.cos(angle + Math.PI / 6), toy - headlen * Math.sin(angle + Math.PI / 6));
                 context.stroke();
               }
            Column {
                id: grid_col1
                width: (main_window.width-24)/3
                Label {
                    id:  powersupply_meter
                    text: "232V / 50 Hz"
                    font.pointSize: 10
                    font.weight: Font.Bold
                    horizontalAlignment: Text.AlignHCenter
                    verticalAlignment: Text.AlignVCenter
                    width: grid_col1.width
                    }
                Image {
                    id:  netz_image
                    source: "qrc:/res/Netz.png"
                    antialiasing: true
                    fillMode: Image.PreserveAspectFit
                    cache: false
                    width: grid_col1.width
                    height: width
                    }
                 Label {
                    id: powermeter_netz_to_house
                    text: "|\n0.00 kW\n▼"
                    font.pointSize: 10
                    font.weight: Font.Bold
                    horizontalAlignment: Text.AlignHCenter
                    verticalAlignment: Text.AlignVCenter
                    width: grid_col1.width
                    height: netz_image.height/3
                    }
                 Image {
                     source: "qrc:/res/Haus.png"
                     antialiasing: true
                     fillMode: Image.PreserveAspectFit
                     cache: false
                     width: grid_col1.width
                     height: width
                     }
                 Label {
                     id:  powermeter_house
                     text: "0.00 kW"
                     font.pointSize: 10
                     font.weight: Font.Bold
                     horizontalAlignment: Text.AlignHCenter
                     verticalAlignment: Text.AlignVCenter
                     width: grid_col1.width
                     }
                }
            Column {
                }
            Column {
                id: grid_col3
                width: (main_window.width-24)/3
                Label { text: " " } // Dummy label
                Label {
                    id: powermeter_panels_to_netz
                    text: "◄ 0.00 kW -"
                    font.pointSize: 10
                    font.weight: Font.Bold
                    horizontalAlignment: Text.AlignHCenter
                    verticalAlignment: Text.AlignVCenter
                    width: grid_col3.width
                    height: netz_image.height
                    }
                Label {
                    id: powermeter_panels_to_house
                    text: "◄- 0.00 kW -◄"
                    font.pointSize: 10
                    font.weight: Font.Bold
                    horizontalAlignment: Text.AlignHCenter
                    verticalAlignment: Text.AlignVCenter
                    width: grid_col3.width
                    height: netz_image.height/3
                    transformOrigin: Item.Center
                    rotation: -30
                    }
                Label {
                    id: powermeter_sonnenbatterie_to_house
                    text: "◄ 0.00 kW -"
                    font.pointSize: 10
                    font.weight: Font.Bold
                    horizontalAlignment: Text.AlignHCenter
                    verticalAlignment: Text.AlignVCenter
                    width: grid_col3.width
                    height: netz_image.height
                    }
                Label { text: " " } // Dummy label
                }
            Column {
                }
            Column {
                id: grid_col5
                width: (main_window.width-24)/3
                Label {
                    id:  powermeter_panels
                    text: "0.00 kW"
                    font.pointSize: 10
                    font.weight: Font.Bold
                    horizontalAlignment: Text.AlignHCenter
                    verticalAlignment: Text.AlignVCenter
                    width: grid_col5.width
                    }
                Image {
                    source: "qrc:/res/Panels.png"
                    antialiasing: true
                    fillMode: Image.PreserveAspectFit
                    cache: false
                    width: grid_col5.width
                    height: width
                    }
                Label {
                   id: powermeter_panels_to_sonnenbatterie
                   text: "|\n0.00 kW\n▼"
                   font.pointSize: 10
                   font.weight: Font.Bold
                   horizontalAlignment: Text.AlignHCenter
                   verticalAlignment: Text.AlignVCenter
                   width: grid_col1.width
                   height: netz_image.height/3
                   }
                Image {
                    source: "qrc:/res/SonnenBatterie.png"
                    antialiasing: true
                    fillMode: Image.PreserveAspectFit
                    cache: false
                    width: grid_col5.width
                    height: width
                    }
                Label {
                    id:  energymeter_sonnenbatterie
                    text: "0.00 kWh / 0 %"
                    font.pointSize: 10
                    font.weight: Font.Bold
                    color: "black"
                    horizontalAlignment: Text.AlignHCenter
                    verticalAlignment: Text.AlignVCenter
                    width: grid_col5.width
                    }
                }
            }
        Row {
            height: 20
            padding: 10
            bottomPadding: 0
            Text {
                id: label_version
                text: "Version: yyyy-MM-dd"
                font.pointSize: 7
                }
            }
        Row {
            id: access_config_file
            height: 20
            padding: 10
            topPadding: 0

            Text {
                id: label_config_file
                text: "<filename of config.>"
                font.pointSize: 7
                width: main_window.width-120
                anchors.bottom: menu_button.bottom
                }
            RoundButton {
                id: menu_button
                height: 20
                radius: 20
                //flat: true
                checkable: true
                text: "..."
                onToggled: {
                    if (menu_button.checked) {
                        edit_settings.visible = true
                        edit_settings.anchors.top = access_config_file.bottom
                        }
                    else {
                        edit_settings.visible = false
                        }
                    }
                }
            }
        Grid {
            id: edit_settings
            height: 0
            columns: 2
            visible: false
            padding: 10
            Label {
                topPadding: 10
                text: qsTr("IP / Hostname: ")
                }
            TextInput {
                id: edit_ip_addr
                topPadding: 10
                text: "xxx.xxx.xxx.xxx"
                selectByMouse: true
                inputMethodHints: Qt.ImhUrlCharactersOnly
                onEditingFinished: main_column_layout.editingFinished()
                }
            Label {
                topPadding: 10
                text: qsTr("Seriennummer: ")
                }
            TextInput {
                id: edit_serial_number
                topPadding: 10
                text: "nnnnnn"
                validator: IntValidator { bottom: 100000; top: 999999 }
                selectByMouse: true
                inputMethodHints: Qt.ImhDigitsOnly
                onEditingFinished: main_column_layout.editingFinished()
                }
            Label {
                topPadding: 10
                text: "Auth-Token: "
                visible: false
                }
            TextInput {
                id : edit_auth_token
                topPadding: 10
                text: "xxxx-xxxx-xxxx..."
                selectByMouse: true
                visible: false
                onEditingFinished: main_column_layout.editingFinished()
                }
            Label {
                topPadding: 10
                text: qsTr("Speicherkapazität / kWh: ")
                }
            TextInput {
                id : edit_capacity
                topPadding: 10
                text: "10"
                inputMethodHints: Qt.ImhDigitsOnly
                selectByMouse: true
                validator: IntValidator { bottom: 5; top: 20 }
                onEditingFinished: main_column_layout.editingFinished()
                }
            }

        // Connection: QML signal -> C++ slot
        // @see Qt documentation: "Interacting with QML Objects from C++" / "Connecting to QML Signals"
        //                        https://doc.qt.io/qt-6/qtqml-cppintegration-interactqmlfromcpp.html
        signal signalEditingFinished(sn: variant, ip: variant, auth: variant, cap: variant)
        // Invoking the QML signal
        function editingFinished() {
            signalEditingFinished(edit_serial_number.text, edit_ip_addr.text, edit_auth_token.text, edit_capacity.text)
        }
    } // End of main_column_layout

    // The connections between c++ signal and QML slot
    Connections {
        target:  QmlBackend

        // Connections: C++ signals -> QML slots
        function notifyWifiStatusChanged(status) {
            console.log("Wifi status has changed: ", status)
            if (status === 0) {
                wifi.source = "qrc:/res/wifi-disabled.png"
                wifi_led.source = "qrc:/res/led-gr.png"
                }
            if (status === 1) {
                wifi.source = "qrc:/res/wifi-enabled.png"
                wifi_led.source = "qrc:/res/led-rt.png"
                }
            if (status === 2) {
                wifi.source = "qrc:/res/wifi-enabled.png"
                wifi_led.source = "qrc:/res/led-gn.png"
                }
            }
        function notifyPowermeterPanels2NetzChanged(value) {
            //console.log("Powermeter pannels->mains changed")
            powermeter_panels_to_netz.text = "◄- " + value + " kW --"
            }
        function notifyPowermeterNetz2HouseChanged(value) {
            //console.log("Powermeter mains->house changed")
            powermeter_netz_to_house.text = "|\n" + value + " kW\n▼"
            }
        function notifyPowermeterPanels2HouseChanged(value) {
            //console.log("Powermeter panels->house changed")
            //powermeter_panels_to_house.text = "-◄- " +value +" kW -◄-"
            powermeter_panels_to_house.text = "◄--- " + value +" kW ----" // with rotation: -30
            }
        function notifyPowermeterPanels2SonnenBatterieChanged(value) {
            //console.log("Powermeter panels->SonnenBatterie changed")
            powermeter_panels_to_sonnenbatterie.text = "|\n" + value + " kW\n▼"
            }
        function notifyPowermeterSonnenBatterie2HouseChanged(value) {
            //console.log("Powermeter SonnenBatterie->house changed")
            powermeter_sonnenbatterie_to_house.text = "◄- " + value + " kW --"
            }
        function notifyPowermeterPowerConsumptionChanged(value) {
            //console.log("Powermeter power consumption changed")
            powermeter_house.text = value + " kW"
            }
        function notifyEnergymeterSonnenBatterieChanged(energy, percentage) {
            //console.log("Powermeter energy SonnenBatterie changed")
            if (percentage < 10)    energymeter_sonnenbatterie.color = "#FF0000"
            else if (percentage <20)energymeter_sonnenbatterie.color = "darkorange"
            else                    energymeter_sonnenbatterie.color = "black"
            energymeter_sonnenbatterie.text = energy + " kWh / " + percentage + " %"
            }
        function notifyPowermeterPanelsChanged(value) {
            //console.log("Powermeter panels changed")
            powermeter_panels.text = value + " kW"
            }
        function notifyPowersupplyVoltageFrequencyChanged(voltage, frequency) {
            //console.log("Powermeter panels changed")
            powersupply_meter.text = voltage +" V / " + frequency + " Hz"
            }
        function notifySettingsChanged(serialnumber, ip_addr, auth_token, capacity) {
            //console.log("Serial number changed")
            serial_number.text = serialnumber
            edit_serial_number.text = serialnumber
            edit_auth_token.text = auth_token
            edit_ip_addr.text = ip_addr
            edit_capacity.text = capacity
            }
        function notifyConfigLabelChanged(filename) {
            label_config_file.text = filename
            }
        function notifyVersionLabelChanged(version) {
            label_version.text = "Version: " + version
            }
    } // End of Connections


} // End of Window
