import QtQuick
import QtQuick.Controls
import QtQuick.Layouts

Page {
    ScrollView {
        anchors.fill: parent
        contentWidth: availableWidth

        ColumnLayout {
            width: parent.width
            spacing: 20

            Item { Layout.preferredHeight: 24 }

            Label {
                text: "Backtest"
                font.pixelSize: 24
                font.bold: true
                color: "#ffffff"
                Layout.leftMargin: 24
            }

            RowLayout {
                Layout.leftMargin: 24
                spacing: 12

                Label { text: "Months:"; color: "#b0b0b0" }
                SpinBox {
                    id: monthsInput
                    from: 3
                    to: 120
                    value: 12
                    editable: true
                }

                Button {
                    text: "Run Backtest"
                    highlighted: true
                    enabled: portfolioController.portfolioResult.holdings !== undefined
                    onClicked: backtestController.runBacktest(portfolioController.portfolioResult.holdings, monthsInput.value)
                }

                BusyIndicator {
                    running: backtestController.isRunning
                    visible: running
                    Layout.preferredWidth: 32
                    Layout.preferredHeight: 32
                }
            }

            Label {
                text: "⚠️ Generate a portfolio first to run backtests."
                color: "#ff9800"
                visible: portfolioController.portfolioResult.holdings === undefined
                Layout.leftMargin: 24
            }

            // Results
            Frame {
                Layout.fillWidth: true
                Layout.leftMargin: 24
                Layout.rightMargin: 24
                Layout.minimumHeight: 300
                visible: backtestController.result.totalReturn !== undefined

                ColumnLayout {
                    anchors.fill: parent
                    spacing: 16

                    RowLayout {
                        spacing: 24
                        Label {
                            text: "📈 Total Return: " + (backtestController.result.totalReturn || 0).toFixed(2) + "%"
                            font.pixelSize: 16
                            color: (backtestController.result.totalReturn || 0) >= 0 ? "#4ecdc4" : "#ff6b6b"
                        }
                        Label {
                            text: "📊 Annualized: " + (backtestController.result.annualizedReturn || 0).toFixed(2) + "%"
                            font.pixelSize: 16
                            color: "#ffe66d"
                        }
                        Label {
                            text: "📉 Max Drawdown: -" + (backtestController.result.maxDrawdown || 0).toFixed(2) + "%"
                            font.pixelSize: 16
                            color: "#ff6b6b"
                        }
                    }

                    // Monthly returns table
                    Rectangle {
                        Layout.fillWidth: true
                        height: 32
                        color: "#2a2d35"
                        radius: 4
                        RowLayout {
                            anchors.fill: parent
                            anchors.margins: 8
                            Label { text: "Month"; font.bold: true; Layout.preferredWidth: 80; color: "#a0a0a0" }
                            Label { text: "Return (%)"; font.bold: true; Layout.preferredWidth: 120; color: "#a0a0a0" }
                            Label { text: "Cumulative (%)"; font.bold: true; Layout.preferredWidth: 140; color: "#a0a0a0" }
                            Item { Layout.fillWidth: true }
                        }
                    }

                    ListView {
                        id: backtestList
                        Layout.fillWidth: true
                        Layout.preferredHeight: contentHeight
                        Layout.maximumHeight: 300
                        clip: true
                        interactive: true
                        model: backtestController.result.monthlyData || []
                        delegate: Rectangle {
                            required property var modelData
                            required property int index
                            width: backtestList.width
                            height: 36
                            color: index % 2 === 0 ? "#22252d" : "#1e2128"
                            RowLayout {
                                anchors.fill: parent
                                anchors.margins: 8
                                Label { text: modelData.month; Layout.preferredWidth: 80; color: "#e0e0e0" }
                                Label { text: modelData["return"].toFixed(2); Layout.preferredWidth: 120; color: modelData["return"] >= 0 ? "#4ecdc4" : "#ff6b6b" }
                                Label { text: modelData.cumulative.toFixed(2); Layout.preferredWidth: 140; color: modelData.cumulative >= 0 ? "#4ecdc4" : "#ff6b6b" }
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
