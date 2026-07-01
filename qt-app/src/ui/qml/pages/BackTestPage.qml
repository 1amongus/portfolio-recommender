import QtQuick
import QtQuick.Controls
import QtQuick.Layouts

Page {
    id: page
    property int pageIndex: 1

    function percentText(value) {
        return (Number(value || 0) * 100).toFixed(2) + "%"
    }

    function curveMax(curve) {
        if (!curve || curve.length === 0) {
            return 1.0
        }

        var maxValue = curve[0]
        for (var i = 1; i < curve.length; ++i) {
            if (curve[i] > maxValue) {
                maxValue = curve[i]
            }
        }
        return Math.max(maxValue, 1.0)
    }

    header: ToolBar {
        contentHeight: 48
        Label {
            anchors.verticalCenter: parent.verticalCenter
            anchors.left: parent.left
            anchors.leftMargin: 16
            text: "Back-Test"
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
                    text: "3.50"
                    placeholderText: "e.g. 3.50"
                    validator: DoubleValidator { bottom: 0.0; top: 100.0; decimals: 2 }
                    Layout.preferredWidth: 120
                }

                Label {
                    text: "Years"
                    Layout.alignment: Qt.AlignVCenter
                }

                ComboBox {
                    id: yearsComboBox
                    model: [1, 3, 5, 10]
                    Layout.preferredWidth: 100
                }

                Button {
                    text: "Run Back-Test"
                    onClicked: backtestController.runBacktest(
                        Number.fromLocaleString(Qt.locale(), targetYieldField.text) / 100.0,
                        Number(yearsComboBox.currentText)
                    )
                }

                BusyIndicator {
                    running: backtestController.isLoading
                    visible: running
                }
            }

            Label {
                text: backtestController.errorMessage
                color: "#ff8a80"
                visible: backtestController.errorMessage.length > 0
                wrapMode: Text.WordWrap
                Layout.fillWidth: true
            }

            Frame {
                Layout.fillWidth: true
                visible: !backtestController.isLoading && (backtestController.result.equityCurve || []).length > 0

                ColumnLayout {
                    anchors.fill: parent
                    spacing: 16

                    Label {
                        text: "Period: " + (backtestController.result.startDate || "-") + " → " + (backtestController.result.endDate || "-")
                        color: "#cfd8dc"
                    }

                    GridLayout {
                        Layout.fillWidth: true
                        columns: width > 720 ? 5 : 2
                        rowSpacing: 12
                        columnSpacing: 12

                        Repeater {
                            model: [
                                { title: "Total Return", value: page.percentText(backtestController.result.metrics ? backtestController.result.metrics.totalReturn : 0) },
                                { title: "Annualized Return", value: page.percentText(backtestController.result.metrics ? backtestController.result.metrics.annualizedReturn : 0) },
                                { title: "Max Drawdown", value: page.percentText(backtestController.result.metrics ? backtestController.result.metrics.maxDrawdown : 0) },
                                { title: "Sharpe", value: Number(backtestController.result.metrics ? backtestController.result.metrics.sharpeRatio : 0).toFixed(2) },
                                { title: "Sortino", value: Number(backtestController.result.metrics ? backtestController.result.metrics.sortinoRatio : 0).toFixed(2) }
                            ]

                            delegate: Frame {
                                Layout.fillWidth: true
                                background: Rectangle {
                                    color: "#20242c"
                                    radius: 8
                                    border.color: "#2e3440"
                                }

                                Column {
                                    spacing: 6

                                    Label {
                                        text: modelData.title
                                        color: "#9fb3c8"
                                    }

                                    Label {
                                        text: modelData.value
                                        font.pixelSize: 20
                                        font.bold: true
                                    }
                                }
                            }
                        }
                    }

                    ColumnLayout {
                        Layout.fillWidth: true
                        spacing: 8

                        Label {
                            text: "Equity Curve"
                            font.pixelSize: 18
                            font.bold: true
                        }

                        Label {
                            text: "Chart: [" + (backtestController.result.equityCurve || []).slice(0, 10).map(function(v) { return Number(v).toFixed(2) }).join(", ")
                                  + ((backtestController.result.equityCurve || []).length > 10 ? ", ..." : "") + "]"
                            color: "#9fb3c8"
                            wrapMode: Text.WrapAnywhere
                            Layout.fillWidth: true
                        }

                        Flickable {
                            Layout.fillWidth: true
                            Layout.preferredHeight: 180
                            clip: true
                            contentWidth: equityBars.implicitWidth

                            Row {
                                id: equityBars
                                spacing: 2
                                anchors.bottom: parent.bottom

                                Repeater {
                                    model: backtestController.result.equityCurve || []

                                    delegate: Rectangle {
                                        required property double modelData
                                        width: 4
                                        height: Math.max(6, (modelData / page.curveMax(backtestController.result.equityCurve || [])) * 150)
                                        radius: 2
                                        color: "#4fc3f7"
                                        anchors.bottom: parent.bottom
                                    }
                                }
                            }
                        }
                    }
                }
            }

            Label {
                text: backtestController.isLoading ? "Running back-test..." : "Enter a target yield and duration to simulate performance."
                color: "#9fb3c8"
                visible: backtestController.isLoading || (backtestController.result.equityCurve || []).length === 0
            }
        }
    }
}