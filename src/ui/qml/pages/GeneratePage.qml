import QtQuick
import QtQuick.Controls
import QtQuick.Layouts

Page {
    id: page

    ScrollView {
        anchors.fill: parent
        contentWidth: availableWidth

        ColumnLayout {
            width: parent.width
            spacing: 20

            Item { Layout.preferredHeight: 24 }

            // Header
            Label {
                text: "Generate Portfolio"
                font.pixelSize: 24
                font.bold: true
                color: "#ffffff"
                Layout.leftMargin: 24
            }

            // Controls row
            RowLayout {
                Layout.leftMargin: 24
                Layout.rightMargin: 24
                spacing: 12

                Label {
                    text: "Target Yield (%)"
                    color: "#b0b0b0"
                    Layout.alignment: Qt.AlignVCenter
                }

                TextField {
                    id: targetYieldField
                    text: (portfolioController.targetYield * 100).toFixed(2)
                    placeholderText: "e.g. 3.50"
                    implicitWidth: 100
                    validator: DoubleValidator { bottom: 0.0; top: 100.0; decimals: 2 }
                }

                Button {
                    text: "🚀 Generate"
                    highlighted: true
                    onClicked: {
                        var val = parseFloat(targetYieldField.text)
                        if (!isNaN(val) && val > 0) {
                            portfolioController.setTargetYield(val / 100.0)
                        }
                        portfolioController.generate()
                    }
                }

                BusyIndicator {
                    running: portfolioController.isLoading
                    visible: running
                    Layout.preferredWidth: 32
                    Layout.preferredHeight: 32
                }
            }

            // Error message
            Label {
                text: portfolioController.errorMessage
                color: "#ff6b6b"
                visible: portfolioController.errorMessage.length > 0
                Layout.leftMargin: 24
                wrapMode: Text.WordWrap
                Layout.fillWidth: true
            }

            // Results frame
            Frame {
                Layout.fillWidth: true
                Layout.leftMargin: 24
                Layout.rightMargin: 24
                Layout.minimumHeight: 350
                visible: portfolioController.portfolioResult.holdings !== undefined

                ColumnLayout {
                    anchors.fill: parent
                    spacing: 12

                    // Summary row
                    RowLayout {
                        Layout.fillWidth: true
                        spacing: 24

                        Label {
                            text: "✅ Achieved Yield: " + ((portfolioController.portfolioResult.achievedYield || 0) * 100).toFixed(2) + "%"
                            font.pixelSize: 14
                            color: "#4ecdc4"
                        }
                        Label {
                            text: "📉 Portfolio Beta: " + Number(portfolioController.portfolioResult.aggregateBeta || 0).toFixed(3)
                            font.pixelSize: 14
                            color: "#ffe66d"
                        }
                    }

                    // Table header
                    Rectangle {
                        Layout.fillWidth: true
                        height: 40
                        color: "#2a2d35"
                        radius: 4

                        RowLayout {
                            anchors.fill: parent
                            anchors.margins: 8

                            Label { text: "Ticker"; font.bold: true; Layout.preferredWidth: 100; color: "#a0a0a0" }
                            Label { text: "Weight"; font.bold: true; Layout.preferredWidth: 100; color: "#a0a0a0" }
                            Label { text: "Yield"; font.bold: true; Layout.preferredWidth: 100; color: "#a0a0a0" }
                            Label { text: "Beta"; font.bold: true; Layout.preferredWidth: 100; color: "#a0a0a0" }
                            Item { Layout.fillWidth: true }
                        }
                    }

                    // Holdings list
                    ListView {
                        id: holdingsView
                        Layout.fillWidth: true
                        Layout.preferredHeight: contentHeight
                        Layout.minimumHeight: 100
                        interactive: false
                        clip: true
                        model: portfolioController.portfolioResult.holdings || []

                        delegate: Rectangle {
                            required property var modelData
                            required property int index
                            width: holdingsView.width
                            height: 44
                            color: index % 2 === 0 ? "#22252d" : "#1e2128"

                            RowLayout {
                                anchors.fill: parent
                                anchors.margins: 8

                                Label { text: modelData.ticker; Layout.preferredWidth: 100; color: "#ffffff"; font.bold: true }
                                Label { text: (modelData.weight * 100).toFixed(1) + "%"; Layout.preferredWidth: 100; color: "#e0e0e0" }
                                Label { text: (modelData["yield"] * 100).toFixed(2) + "%"; Layout.preferredWidth: 100; color: "#4ecdc4" }
                                Label { text: Number(modelData.beta).toFixed(3); Layout.preferredWidth: 100; color: "#ffe66d" }
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
