import QtQuick 2.0
import QtQuick.Controls 1.4

import QtWebEngine 1.3
import QtQuick.Layouts 1.3
import QtWebChannel 1.0


TabView {
    id: mainPanel
    width: parent.width
    height: parent.height

    Tab {
        id: mapTab
        title: "Map"

        WebEngineView {
            id: map
            anchors.fill: parent
            url: "qrc:/web/map.html"
            webChannel: WebChannel {
                registeredObjects: [gps]
            }
        }
    }
    Tab {
        id: settingsTab
        title: "Settings"
        source: "SettingsView.qml"
    }
    Tab {
        id: dataTab
        title: "Data"
        source: "DataView.qml"

    }

}

