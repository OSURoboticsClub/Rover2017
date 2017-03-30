import QtQuick 2.0
import QtQuick.Controls 2.0
import QtPositioning 5.7
import QtQuick.Extras 1.4
import QtQuick.Layouts 1.3
import QtQuick.Controls.Styles 1.4

import QtQuick.Templates 2.0
import QtQuick.Controls 1.1
import QtQuick.Controls 1.0
import QtQuick.Controls 1.4


Item {
    property int t
    id: item1
    width: 200
    height: 600



    Item {
        id: voltageGuage
        property int min: 20
        property int max: 25
        property double value: root.battery_voltage/1000
        property color color: "black"
        property double fakeVoltage: (parent.width) * ((voltageGuage.value - voltageGuage.min)/(voltageGuage.max - voltageGuage.min));


        width: parent.width; height: 37
        Rectangle {
            id: border
            height: 51
            anchors.fill: parent
            anchors.bottomMargin: 2
            anchors.rightMargin: 2
            color: "transparent"
            border.width: 2
            border.color: parent.color
        }
        Rectangle {
            id: progress
            anchors.top: border.top
            anchors.bottom: border.bottom
            anchors.left: border.left
            anchors.margins: 2
            width: parent.fakeVoltage
            color: ((voltageGuageLabel.width/2) + (parent.width/2)) <= (progress.width) ? "blue" : "red"
        }

        Text {
            id: voltageGuageLabel
            x: 71
            y: 10
            text: parent.value + "V";
            anchors.verticalCenter: parent.verticalCenter
            anchors.horizontalCenter: parent.horizontalCenter
            font.bold: true
            font.family: "Tahoma"
            font.pixelSize: 13
            color: ((voltageGuageLabel.width/2) + (parent.width/2)) <= (progress.width) ? "white" : "black";
        }
    }

    Image {
        id: logo
        x: 0
        y: 501
        anchors.bottom: parent.bottom
        width: parent.width
        height: 91
        source: "Logo0.png"
        property int logoNumber: 0
        MouseArea {
            anchors.fill: parent
            onClicked: {
                logo.logoNumber++;
                logo.source = "Logo" + (logo.logoNumber % 3) + ".png";
            }
        }
    }
    CircularGauge {
        id: speedGuage
        x: 0
        width: 200
        anchors.bottom: compassHeading.top
        anchors.bottomMargin: -10
        anchors.top: voltageGuage.bottom
        anchors.topMargin: 6
        anchors.horizontalCenter: parent.horizontalCenter
        minimumValue: 0
        maximumValue: 20
        stepSize: 1
        value: root.gps_speed/1000
        Text {
            font.family: "Verdana"
            font.pointSize: 16
            anchors.verticalCenterOffset: -40
            anchors.horizontalCenterOffset: 0
            anchors.verticalCenter: parent.verticalCenter
            anchors.horizontalCenter: parent.horizontalCenter
            text: root.gps_speed/1000 + " KPH"
        }

        style: CircularGaugeStyle {
            minorTickmarkCount: 5
            tickmarkStepSize: 5
            minimumValueAngle: -90
            maximumValueAngle: 90
            needle: Rectangle {
                implicitWidth: outerRadius * 0.03
                implicitHeight: outerRadius * .85
                antialiasing: true
                color: "black"
            }
            foreground: Item {
                Rectangle {
                    width: outerRadius * 0.2
                    height: width
                    radius: width / 2
                    color: "black"
                    anchors.centerIn: parent
                }
            }
            tickmarkLabel: Text {
                font.pixelSize: Math.max(6, outerRadius * 0.1)
                text: styleData.value
                color: styleData.value >= (speedGuage.maximumValue * .8) ? "red" : "green"
                antialiasing: true
            }
            tickmark: Rectangle {
                visible: styleData.value < speedGuage.maximumValue || styleData.value % 10 == 0
                implicitWidth: outerRadius * 0.02
                antialiasing: true
                implicitHeight: outerRadius * 0.11
                color: styleData.value >= (speedGuage.maximumValue * .8) ? "red" : "green"
            }
            minorTickmark: Rectangle {
                visible: styleData.value < speedGuage.maximumValue
                implicitWidth: outerRadius * .01
                antialiasing: true
                implicitHeight: outerRadius * .03
                color: styleData.value >= (speedGuage.maximumValue * .8) ? "red" : "green"
            }
        }
    }
    CircularGauge {
        id: compassHeading
        x: -12
        width: 200
        anchors.horizontalCenterOffset: 0
        anchors.horizontalCenter: parent.horizontalCenter
        anchors.top: voltageGuage.bottom
        anchors.topMargin: 246
        minimumValue: 0
        maximumValue: 360
        value: root.gps_heading
        Text {
            id: compassHeadingText
            anchors.horizontalCenter: parent.horizontalCenter
            anchors.top: parent.bottom
            anchors.topMargin: 10
            text: root.gps_heading + " deg"
            font.family: "Verdana"
            font.pointSize: 16
        }

        function colorChooser(value, min, max){
            var range = max - min;
            if (range * .25 > value){
                //orangish
                return "#552100";
            }
            else if (range * .5 > value){
                //brownish
                return "#553500";
            }
            else if (range * .75 > value){
                //blueish
                return "#051A38";
            }
            else {
                //green-blueish
                return "#00372B";

            }
        }
        style: CircularGaugeStyle {
            minimumValueAngle: 0
            maximumValueAngle: 360
            minorTickmarkCount: 2
            tickmarkStepSize: 45
            needle: Rectangle {
                implicitWidth: outerRadius * 0.03
                implicitHeight: outerRadius * .74
                antialiasing: true
                color: compassHeading.colorChooser(compassHeading.value, compassHeading.minimumValue, compassHeading.maximumValue);
            }
            foreground: Item {
                Rectangle {
                    width: outerRadius * 0.2
                    height: width
                    radius: width / 2
                    color: compassHeading.colorChooser(compassHeading.value, compassHeading.minimumValue, compassHeading.maximumValue);
                    anchors.centerIn: parent
                }
            }
            tickmarkLabel: Text {
                font.pixelSize: Math.max(6, outerRadius * 0.1)
                text: styleData.value
                color: "black"
                antialiasing: true
                visible: styleData.value < compassHeading.maximumValue
            }
            tickmark: Rectangle {
                visible: styleData.value < compassHeading.maximumValue || styleData.value % 10 == 0
                implicitWidth: outerRadius * 0.02
                antialiasing: true
                implicitHeight: outerRadius * 0.11
                color: "black"
            }
            minorTickmark: Rectangle {
                visible: styleData.value < speedGuage.maximumValue
                implicitWidth: outerRadius * .01
                antialiasing: true
                implicitHeight: outerRadius * .03
                color: "black"
            }
        }
    }
}
