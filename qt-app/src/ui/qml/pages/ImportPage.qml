import QtQuick
import QtQuick.Controls
import QtQuick.Layouts

Page {
    id: page
    property int pageIndex: 3

    function formatPercent(value) {
        return (Number(value || 0) * 100).toFixed(2) + "%"
    }

    function signedPercent(value) {
        const number = Number(value || 0) * 100
        return (number >= 0 ? "+" : "") + number.toFixed(2) + "%"
    }

    function signedNumber(value) {
        const number = Number(value || 0)
        return (number >= 0 ? "+" : "") + number.toFixed(3)
    }

    function comparisonColor(originalValue, updatedValue, lowerIsBetter) {
        return lowerIsBetter
            ? (updatedValue <= originalValue ? "#81c784" : "#ef9a9a")
            : (updatedValue >= originalValue ? "#81c784" : "#ef9a9a")
    }

    header: ToolBar {
        contentHeight: 48
        Label {
            anchors.verticalCenter: parent.verticalCenter
            anchors.left: parent.left
            anchors.leftMargin: 16
            text: "Import & Rebalance"
            font.pixelSize: 22
            font.bold: true
        }
    }

    ScrollView {
        anchors.fill: parent
        contentWidth: availableWidth

        RowLayout {
            width: parent.width
            spacing: 20
            padding: 24

            Frame {
                Layout.fillWidth: true
                Layout.fillHeight: true

                ColumnLayout {
                    anchors.fill: parent
                    spacing: 16

                    Label {
                        text: "Import Portfolio"
                        font.bold: true
                        font.pixelSize: 18
                    }

                    RowLayout {
                        Layout.fillWidth: true
                        spacing: 8

                        TextField {
                            id: filePathField
                            Layout.fillWidth: true
                            placeholderText: "Q:\\path\\to\\portfolio.csv"
                        }

                        Button {
                            text: "Import CSV"
                            onClicked: importController.importFromFile(filePathField.text)
                        }
                    }

                    ColumnLayout {
                        Layout.fillWidth: true
                        spacing: 8

                        Label { text: "Paste CSV" }
                        TextArea {
                            id: csvTextArea
                            Layout.fillWidth: true
                            Layout.preferredHeight: 140
                            wrapMode: TextArea.NoWrap
                            placeholderText: "ticker,weight\nSCHD,0.50\nVYM,0.50"
                        }

                        Button {
                            text: "Parse"
                            Layout.alignment: Qt.AlignRight
                            onClicked: importController.importFromText(csvTextArea.text)
                        }
                    }

                    RowLayout {
                        Layout.fillWidth: true
                        spacing: 8

                        TextField {
                            id: tickerField
                            Layout.fillWidth: true
                            placeholderText: "Ticker"
                        }

                        TextField {
                            id: weightField
                            Layout.preferredWidth: 120
                            placeholderText: "Weight"
                            validator: DoubleValidator { bottom: 0.0; top: 1.0; decimals: 4 }
                        }

                        Button {
                            text: "Add"
                            onClicked: {
                                importController.addHolding(tickerField.text, Number.fromLocaleString(Qt.locale(), weightField.text))
                                tickerField.text = ""
                                weightField.text = ""
                            }
                        }
                    }

                    BusyIndicator {
                        running: importController.isLoading
                        visible: running
                        Layout.alignment: Qt.AlignLeft
                    }

                    Label {
                        text: importController.errorMessage
                        color: "#ef9a9a"
                        visible: importController.errorMessage.length > 0
                        wrapMode: Text.Wrap
                        Layout.fillWidth: true
                    }

                    Label {
                        text: "Imported Holdings"
                        font.bold: true
                    }

                    RowLayout {
                        Layout.fillWidth: true
                        Label { text: "Ticker"; font.bold: true; Layout.preferredWidth: 110 }
                        Label { text: "Weight"; font.bold: true; Layout.preferredWidth: 100 }
                        Label { text: "Yield"; font.bold: true; Layout.preferredWidth: 100 }
                        Label { text: "Beta"; font.bold: true; Layout.preferredWidth: 100 }
                        Item { Layout.fillWidth: true }
                    }

                    ListView {
                        Layout.fillWidth: true
                        Layout.preferredHeight: 260
                        clip: true
                        model: importController.importedHoldings
                        delegate: Rectangle {
                            width: ListView.view.width
                            height: 48
                            color: index % 2 === 0 ? "#252a33" : "#20242c"

                            RowLayout {
                                anchors.fill: parent
                                anchors.margins: 10
                                Label { text: modelData.ticker; Layout.preferredWidth: 110 }
                                Label { text: page.formatPercent(modelData.weight); Layout.preferredWidth: 100 }
                                Label { text: page.formatPercent(modelData.yield); Layout.preferredWidth: 100 }
                                Label { text: Number(modelData.beta).toFixed(3); Layout.preferredWidth: 100 }
                                Item { Layout.fillWidth: true }
                                Button {
                                    text: "Remove"
                                    onClicked: importController.removeHolding(index)
                                }
                            }
                        }
                    }

                    Button {
                        text: "Clear"
                        Layout.alignment: Qt.AlignRight
                        onClicked: importController.clear()
                    }

                    Frame {
                        Layout.fillWidth: true
                        visible: importController.parseWarnings.length > 0

                        ColumnLayout {
                            anchors.fill: parent
                            spacing: 6

                            Label {
                                text: "Warnings"
                                font.bold: true
                            }

                            Repeater {
                                model: importController.parseWarnings
                                delegate: Label {
                                    text: "\u2022 " + modelData
                                    wrapMode: Text.Wrap
                                    color: "#ffd180"
                                    Layout.fillWidth: true
                                }
                            }
                        }
                    }
                }
            }

            Frame {
                Layout.fillWidth: true
                Layout.fillHeight: true

                ColumnLayout {
                    anchors.fill: parent
                    spacing: 16

                    Label {
                        text: "Rebalance"
                        font.bold: true
                        font.pixelSize: 18
                    }

                    RowLayout {
                        Layout.fillWidth: true
                        spacing: 8

                        Label {
                            text: "Target Yield"
                            Layout.alignment: Qt.AlignVCenter
                        }

                        TextField {
                            id: targetYieldField
                            Layout.preferredWidth: 120
                            text: Number(importController.targetYield * 100).toFixed(2)
                            validator: DoubleValidator { bottom: 0.0; top: 100.0; decimals: 2 }
                            onEditingFinished: importController.targetYield = Number.fromLocaleString(Qt.locale(), text) / 100.0
                        }

                        Button {
                            text: "Rebalance"
                            onClicked: {
                                importController.targetYield = Number.fromLocaleString(Qt.locale(), targetYieldField.text) / 100.0
                                importController.rebalance()
                            }
                        }
                    }

                    Label {
                        text: "Rebalanced Holdings"
                        font.bold: true
                    }

                    RowLayout {
                        Layout.fillWidth: true
                        Label { text: "Ticker"; font.bold: true; Layout.preferredWidth: 120 }
                        Label { text: "Weight"; font.bold: true; Layout.preferredWidth: 100 }
                        Label { text: "Yield"; font.bold: true; Layout.preferredWidth: 100 }
                        Label { text: "Beta"; font.bold: true; Layout.preferredWidth: 100 }
                    }

                    ListView {
                        Layout.fillWidth: true
                        Layout.preferredHeight: 260
                        clip: true
                        model: importController.rebalancedHoldings
                        delegate: Rectangle {
                            width: ListView.view.width
                            height: 48
                            color: index % 2 === 0 ? "#252a33" : "#20242c"

                            RowLayout {
                                anchors.fill: parent
                                anchors.margins: 10
                                Label { text: modelData.ticker; Layout.preferredWidth: 120 }
                                Label { text: page.formatPercent(modelData.weight); Layout.preferredWidth: 100 }
                                Label { text: page.formatPercent(modelData.yield); Layout.preferredWidth: 100 }
                                Label { text: Number(modelData.beta).toFixed(3); Layout.preferredWidth: 100 }
                            }
                        }
                    }

                    Frame {
                        Layout.fillWidth: true
                        visible: importController.rebalancedHoldings.length > 0

                        ColumnLayout {
                            anchors.fill: parent
                            spacing: 10

                            Label {
                                text: "Comparison"
                                font.bold: true
                            }

                            RowLayout {
                                Layout.fillWidth: true
                                Label {
                                    text: "Beta"
                                    font.bold: true
                                    Layout.preferredWidth: 90
                                }
                                Label {
                                    text: Number(importController.comparison.originalBeta || 0).toFixed(3)
                                            + " \u2192 "
                                            + Number(importController.comparison.rebalancedBeta || 0).toFixed(3)
                                    Layout.fillWidth: true
                                }
                                Label {
                                    text: page.signedNumber(-1 * Number(importController.comparison.betaReduction || 0))
                                    color: page.comparisonColor(
                                               Number(importController.comparison.originalBeta || 0),
                                               Number(importController.comparison.rebalancedBeta || 0),
                                               true)
                                }
                            }

                            RowLayout {
                                Layout.fillWidth: true
                                Label {
                                    text: "Yield"
                                    font.bold: true
                                    Layout.preferredWidth: 90
                                }
                                Label {
                                    text: page.formatPercent(importController.comparison.originalYield || 0)
                                            + " \u2192 "
                                            + page.formatPercent(importController.comparison.rebalancedYield || 0)
                                    Layout.fillWidth: true
                                }
                                Label {
                                    text: page.signedPercent(importController.comparison.yieldDelta || 0)
                                    color: page.comparisonColor(
                                               Number(importController.comparison.originalYield || 0),
                                               Number(importController.comparison.rebalancedYield || 0),
                                               false)
                                }
                            }
                        }
                    }
                }
            }
        }
    }
}
