import QtQuick 2.0

import QtWebEngine 1.3
import QtWebChannel 1.0

import QtQuick.Controls 2.0
import QtQuick.Layouts 1.3
import QtQuick.Controls.Material 2.0


ColumnLayout {
    id: mapView
        //anchors.margins: 10


    WebEngineView {
        id: map

        Layout.fillHeight: true
        Layout.fillWidth: true
        Layout.minimumWidth: 200
        Layout.preferredWidth: 300
        url: "qrc:/web/map.html"


        QtObject {
            id: gps
            property var coords: [root.latitude, root.longitude, root.gps_heading]
            WebChannel.id: "gps"
        }

        webChannel: WebChannel {
            registeredObjects: [gps]

        }
    }
    Row {
        id: mapInput
        Layout.fillWidth: true
        Layout.minimumHeight: 50
        Layout.preferredHeight: 50
        spacing: 10
        TextField {
            id: inputLatitude
            placeholderText: "latitude"
            validator: DoubleValidator{}
        }
        TextField {
            id: inputLongitude
            placeholderText: "longitude"
            validator: DoubleValidator{}
        }
        TextField {
            id: inputName
            placeholderText: "name"
        }
        Button {
            id: submitButton
            text: "Add Waypoint"
        }
    }
}


