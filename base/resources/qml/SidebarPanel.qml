import QtQuick 2.0
import QtQuick.Extras 1.4
import QtQuick.Layouts 1.3
import QtQuick.Controls.Styles 1.4

import QtPositioning 5.7

Rectangle {
    color: "white"

    ColumnLayout {
        id: sidebarPanel
        anchors.fill: parent

        AttitudeIndicator{
            id: indicator
//            width: 100
//            height: 100
            Layout.minimumHeight: 200
            Layout.fillWidth: true
        }

        CircularGauge {
            maximumValue: 20
            Layout.preferredHeight: 200
            Layout.fillWidth: true

            style: CircularGaugeStyle {
                minorTickmarkCount: 5
                tickmarkStepSize: 5
            }
        }


        Text {
            id: text1
            text: "Arm Motors"
            Layout.fillWidth: true
            horizontalAlignment: Text.AlignHCenter
            font.pixelSize: 12
        }

        MultiGauge {
            id: armMotorGauge
            Layout.minimumHeight: 70
            Layout.fillWidth: true
            limits: [-127, 127]
            model: [-50, 100, root.arm_motor_3,
                root.arm_motor_4, root.arm_motor_5]
        }

        Text {
            id: text2
            text: "Motor Power"
            Layout.fillWidth: true
            horizontalAlignment: Text.AlignHCenter
            font.pixelSize: 12
        }

        MultiGauge {
            id: motorPowerGauge
            Layout.minimumHeight: 70
            Layout.fillWidth: true
            limits: [0, 255]
            model: [10, 20, 150,
                root.r_f_drive, root.r_m_drive, root.r_b_drive]
        }

        Rectangle {
            id: voltageGauge
            property int limit: 25
            property double value: root.battery_voltage / 1000
            Layout.fillWidth: true
            Layout.minimumHeight: 40

            color: "transparent"
            border.width: 2

            Rectangle {
                id: progress
                anchors.top: parent.top
                anchors.bottom: parent.bottom
                anchors.left: parent.left
                width: parent.value * parent.width / parent.limit
                color: parent.width/2 <= progress.width ? "blue" : "red"
            }

            Text {
                text: parent.value + " V"
                anchors.verticalCenter: parent.verticalCenter
                anchors.horizontalCenter: parent.horizontalCenter
            }

        }

        Item {
            id: row
            Layout.minimumHeight: 70
            Layout.fillWidth: true

            StatusIndicator {
                id: statusIndicator
                anchors.left: parent.left
                anchors.leftMargin: 0
                anchors.bottom: parent.bottom
                anchors.bottomMargin: 0
            }

            Text {
                id: text3
                text: "Status"
                anchors.verticalCenter: statusIndicator.verticalCenter
                anchors.left: statusIndicator.right
                anchors.leftMargin: 10
                font.pixelSize: 12
            }

            Rectangle {
                id: rectangle
                width: 40
                height: 40
                color: "#e32929"
                anchors.bottom: parent.bottom
                anchors.bottomMargin: 0
                anchors.right: parent.right
                anchors.rightMargin: 0
            }

            Text {
                id: text4
                text: qsTr("Camera")
                horizontalAlignment: Text.AlignRight
                anchors.verticalCenter: text3.verticalCenter
                anchors.right: rectangle.left
                anchors.rightMargin: 10
                font.pixelSize: 12
            }
        }
    }
}

