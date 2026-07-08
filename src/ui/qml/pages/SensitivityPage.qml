import QtQuick
import QtQuick.Controls
import QtQuick.Layouts

import "../components"

Page {
    ScrollView {
        anchors.fill: parent
        contentWidth: availableWidth

        ColumnLayout {
            width: parent.width
            spacing: 20

            Item { Layout.preferredHeight: 24 }

            Label {
                text: "Sensitivity Analysis"
                font.pixelSize: 24
                font.bold: true
                color: "#ffffff"
                Layout.leftMargin: 24
            }

            RowLayout {
                Layout.leftMargin: 24
                spacing: 12

                Label { text: "Min Yield (%):"; color: "#b0b0b0" }
                TextField {
                    id: minYieldField
                    text: "1.0"
                    implicitWidth: 60
                    validator: DoubleValidator { bottom: 0; top: 100; decimals: 1 }
                }
                Label { text: "Max Yield (%):"; color: "#b0b0b0" }
                TextField {
                    id: maxYieldField
                    text: "10.0"
                    implicitWidth: 60
                    validator: DoubleValidator { bottom: 0; top: 100; decimals: 1 }
                }
                Label { text: "Step (%):"; color: "#b0b0b0" }
                TextField {
                    id: stepField
                    text: "0.5"
                    implicitWidth: 60
                    validator: DoubleValidator { bottom: 0.1; top: 10; decimals: 1 }
                }

                Button {
                    text: "Compute Curve"
                    highlighted: true
                    onClicked: {
                        if (sensitivityController)
                            sensitivityController.computeCurve(
                                parseFloat(minYieldField.text) / 100.0,
                                parseFloat(maxYieldField.text) / 100.0,
                                parseFloat(stepField.text) / 100.0
                            )
                    }
                }

                BusyIndicator {
                    running: sensitivityController ? sensitivityController.isComputing : false
                    visible: running
                    Layout.preferredWidth: 32
                    Layout.preferredHeight: 32
                }
            }

            // Chart
            Frame {
                Layout.fillWidth: true
                Layout.leftMargin: 24
                Layout.rightMargin: 24
                visible: sensitivityController && sensitivityController.curve.length > 0

                ColumnLayout {
                    anchors.fill: parent
                    spacing: 12

                    Label {
                        text: "Yield vs Beta Trade-off (" + (sensitivityController ? sensitivityController.curve.length : 0) + " points)"
                        font.pixelSize: 14
                        color: "#b0b0b0"
                    }

                    LineChart {
                        Layout.fillWidth: true
                        Layout.preferredHeight: 250
                        title: "Portfolio Beta by Target Yield"
                        yLabel: "Beta"
                        lineColor: "#ffe66d"
                        fillColor: "#3d3a1a"
                        showZeroLine: false
                        dataPoints: {
                            if (!sensitivityController || !sensitivityController.curve) return []
                            var pts = []
                            var data = sensitivityController.curve
                            for (var i = 0; i < data.length; i++) {
                                pts.push({x: data[i].targetYield.toFixed(1), y: data[i].beta})
                            }
                            return pts
                        }
                    }

                    // Table
                    Rectangle {
                        Layout.fillWidth: true
                        height: 32
                        color: "#2a2d35"
                        radius: 4
                        RowLayout {
                            anchors.fill: parent
                            anchors.margins: 8
                            Label { text: "Target Yield (%)"; font.bold: true; Layout.preferredWidth: 140; color: "#a0a0a0" }
                            Label { text: "Achieved Yield (%)"; font.bold: true; Layout.preferredWidth: 160; color: "#a0a0a0" }
                            Label { text: "Portfolio Beta"; font.bold: true; Layout.preferredWidth: 120; color: "#a0a0a0" }
                            Item { Layout.fillWidth: true }
                        }
                    }

                    ListView {
                        id: curveList
                        Layout.fillWidth: true
                        Layout.preferredHeight: Math.min(contentHeight, 250)
                        clip: true
                        interactive: true
                        model: sensitivityController ? (sensitivityController.curve || []) : []
                        delegate: Rectangle {
                            required property var modelData
                            required property int index
                            width: curveList.width
                            height: 32
                            color: index % 2 === 0 ? "#22252d" : "#1e2128"
                            RowLayout {
                                anchors.fill: parent
                                anchors.margins: 8
                                Label { text: modelData.targetYield.toFixed(2); Layout.preferredWidth: 140; color: "#e0e0e0"; font.pixelSize: 11 }
                                Label { text: modelData.achievedYield.toFixed(2); Layout.preferredWidth: 160; color: "#4ecdc4"; font.pixelSize: 11 }
                                Label { text: modelData.beta.toFixed(4); Layout.preferredWidth: 120; color: "#ffe66d"; font.pixelSize: 11 }
                                Item { Layout.fillWidth: true }
                            }
                        }
                    }
                }
            }

            Item { Layout.preferredHeight: 24 }
        }
    }
}
