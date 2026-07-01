import QtQuick
import QtQuick.Controls
import QtQuick.Layouts

Page {
    id: page
    property int pageIndex: 0

    header: ToolBar {
        contentHeight: 48
        Label {
            anchors.verticalCenter: parent.verticalCenter
            anchors.left: parent.left
            anchors.leftMargin: 16
            text: "Generate Portfolio"
            font.pixelSize: 22
            font.bold: true
        }
    }

    ScrollView {
        anchors.fill: parent
        contentWidth: availableWidth

        ColumnLayout {
            width: parent.width
            spacing: 20
            padding: 24

            RowLayout {
                spacing: 12
                Label {
                    text: "Target Yield"
                    Layout.alignment: Qt.AlignVCenter
                }
                TextField {
                    id: targetYieldField
                    text: Number(portfolioController.targetYield * 100).toFixed(2)
                    placeholderText: "e.g. 3.50"
                    validator: DoubleValidator { bottom: 0.0; top: 100.0; decimals: 2 }
                    onEditingFinished: portfolioController.targetYield = Number.fromLocaleString(Qt.locale(), text) / 100.0
                }
                Button {
                    text: "Generate Portfolio"
                    onClicked: {
                        portfolioController.targetYield = Number.fromLocaleString(Qt.locale(), targetYieldField.text) / 100.0
                        portfolioController.generate()
                    }
                }
                BusyIndicator {
                    running: portfolioController.isLoading
                    visible: running
                }
            }

            Label {
                text: portfolioController.errorMessage
                color: "#ff8a80"
                visible: portfolioController.errorMessage.length > 0
            }

            Frame {
                Layout.fillWidth: true
                Layout.fillHeight: true

                ColumnLayout {
                    anchors.fill: parent
                    spacing: 12

                    RowLayout {
                        Layout.fillWidth: true
                        Label { text: "Achieved Yield: " + ((portfolioController.portfolioResult.achievedYield || 0) * 100).toFixed(2) + "%" }
                        Label { text: "Portfolio Beta: " + Number(portfolioController.portfolioResult.aggregateBeta || 0).toFixed(3) }
                    }

                    RowLayout {
                        Layout.fillWidth: true
                        Label { text: "Ticker"; font.bold: true; Layout.preferredWidth: 120 }
                        Label { text: "Weight"; font.bold: true; Layout.preferredWidth: 120 }
                        Label { text: "Yield"; font.bold: true; Layout.preferredWidth: 120 }
                        Label { text: "Beta"; font.bold: true; Layout.preferredWidth: 120 }
                    }

                    ListView {
                        Layout.fillWidth: true
                        Layout.fillHeight: true
                        clip: true
                        model: portfolioController.portfolioResult.holdings || []
                        delegate: Rectangle {
                            width: ListView.view.width
                            height: 56
                            color: index % 2 === 0 ? "#252a33" : "#20242c"

                            RowLayout {
                                anchors.fill: parent
                                anchors.margins: 12
                                Label { text: modelData.ticker; Layout.preferredWidth: 120 }
                                Label { text: (modelData.weight * 100).toFixed(2) + "%"; Layout.preferredWidth: 120 }
                                Label { text: (modelData.yield * 100).toFixed(2) + "%"; Layout.preferredWidth: 120 }
                                Label { text: Number(modelData.beta).toFixed(3); Layout.preferredWidth: 120 }
                            }
                        }
                    }
                }
            }
        }
    }
}