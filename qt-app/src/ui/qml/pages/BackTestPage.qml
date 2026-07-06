import QtQuick
import QtQuick.Controls
import QtQuick.Layouts
import QtCharts

Page {
    id: page
    property int pageIndex: 1
    property int selectedChartIndex: 0

    function percentText(value) {
        return (Number(value || 0) * 100).toFixed(2) + "%"
    }

    function activeChartData() {
        if (selectedChartIndex === 1)
            return backtestController.drawdownCurveData()
        if (selectedChartIndex === 2)
            return backtestController.rollingYieldData()
        return backtestController.equityCurveData()
    }

    function displayYValue(point) {
        if (selectedChartIndex === 0)
            return Number(point.y || 0)
        return Number(point.y || 0) * 100
    }

    function updateChart() {
        equitySeries.clear()
        drawdownSeries.clear()
        rollingYieldSeries.clear()

        const data = activeChartData()
        if (!data || data.length === 0) {
            const now = new Date()
            xAxis.min = now
            xAxis.max = now
            yAxis.min = 0
            yAxis.max = 1
            yAxisRight.min = -10
            yAxisRight.max = 0
            return
        }

        var minX = Number(data[0].x)
        var maxX = Number(data[0].x)
        var minY = displayYValue(data[0])
        var maxY = minY

        for (var i = 0; i < data.length; ++i) {
            const point = data[i]
            const x = Number(point.x)
            const y = displayYValue(point)

            minX = Math.min(minX, x)
            maxX = Math.max(maxX, x)
            minY = Math.min(minY, y)
            maxY = Math.max(maxY, y)

            if (selectedChartIndex === 1)
                drawdownSeries.append(x, y)
            else if (selectedChartIndex === 2)
                rollingYieldSeries.append(x, y)
            else
                equitySeries.append(x, y)
        }

        const span = Math.max(Math.abs(maxY - minY), 0.01)
        const padding = span * 0.1
        const axisMin = minY - padding
        const axisMax = maxY + padding

        xAxis.min = new Date(minX)
        xAxis.max = new Date(maxX)

        if (selectedChartIndex === 1) {
            yAxisRight.min = axisMin
            yAxisRight.max = axisMax
        } else {
            yAxis.min = axisMin
            yAxis.max = axisMax
        }
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

    Connections {
        target: backtestController
        function onResultChanged() { page.updateChart() }
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

                Label {
                    text: "Rebalance"
                    Layout.alignment: Qt.AlignVCenter
                }

                ComboBox {
                    id: rebalanceComboBox
                    model: ["Never", "Monthly", "Quarterly", "Annually"]
                    currentIndex: backtestController.rebalanceFrequency
                    Layout.preferredWidth: 150
                    onCurrentIndexChanged: backtestController.rebalanceFrequency = currentIndex
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

                    TabBar {
                        id: chartTabs
                        Layout.fillWidth: true
                        currentIndex: page.selectedChartIndex
                        onCurrentIndexChanged: {
                            page.selectedChartIndex = currentIndex
                            page.updateChart()
                        }

                        TabButton { text: "Equity Curve" }
                        TabButton { text: "Drawdown" }
                        TabButton { text: "Rolling Yield" }
                    }

                    Label {
                        text: selectedChartIndex === 1
                            ? "Drag to zoom. Middle-drag to pan. Right-click to reset."
                            : "Drag to zoom. Middle-drag to pan. Right-click to reset."
                        color: "#9fb3c8"
                    }

                    ChartView {
                        id: equityChart
                        Layout.fillWidth: true
                        Layout.preferredHeight: 400
                        antialiasing: true
                        theme: ChartView.ChartThemeDark
                        legend.visible: true
                        rubberBand: ChartView.RectangleRubberBand

                        DateTimeAxis {
                            id: xAxis
                            format: "yyyy-MM"
                            titleText: "Date"
                            tickCount: 6
                        }

                        ValueAxis {
                            id: yAxis
                            titleText: page.selectedChartIndex === 2 ? "Rolling Yield (%)" : "Portfolio Value"
                            labelFormat: page.selectedChartIndex === 0 ? "%.2f" : "%.2f%%"
                            visible: page.selectedChartIndex !== 1
                        }

                        ValueAxis {
                            id: yAxisRight
                            titleText: "Drawdown (%)"
                            labelFormat: "%.2f%%"
                            alignment: Qt.AlignRight
                            visible: page.selectedChartIndex === 1
                        }

                        LineSeries {
                            id: equitySeries
                            name: "Equity Curve"
                            axisX: xAxis
                            axisY: yAxis
                            color: "#4dd0e1"
                            width: 2
                            visible: page.selectedChartIndex === 0
                        }

                        LineSeries {
                            id: drawdownSeries
                            name: "Drawdown"
                            axisX: xAxis
                            axisY: yAxisRight
                            color: "#ff8a80"
                            width: 1.5
                            visible: page.selectedChartIndex === 1
                        }

                        LineSeries {
                            id: rollingYieldSeries
                            name: "Rolling Yield"
                            axisX: xAxis
                            axisY: yAxis
                            color: "#ffd54f"
                            width: 2
                            visible: page.selectedChartIndex === 2
                        }

                        MouseArea {
                            anchors.fill: parent
                            acceptedButtons: Qt.RightButton | Qt.MiddleButton
                            hoverEnabled: true
                            property real lastX: 0
                            property real lastY: 0

                            onPressed: {
                                lastX = mouse.x
                                lastY = mouse.y
                                if (mouse.button === Qt.RightButton)
                                    equityChart.zoomReset()
                            }

                            onPositionChanged: {
                                if (!(mouse.buttons & Qt.MiddleButton))
                                    return

                                const dx = mouse.x - lastX
                                const dy = mouse.y - lastY

                                if (dx > 0)
                                    equityChart.scrollLeft(dx)
                                else if (dx < 0)
                                    equityChart.scrollRight(-dx)

                                if (dy > 0)
                                    equityChart.scrollDown(dy)
                                else if (dy < 0)
                                    equityChart.scrollUp(-dy)

                                lastX = mouse.x
                                lastY = mouse.y
                            }
                        }
                    }
                }
            }

            Label {
                text: backtestController.isLoading ? "Running back-test..." : "Enter a target yield, duration, and rebalance frequency to simulate performance."
                color: "#9fb3c8"
                visible: backtestController.isLoading || (backtestController.result.equityCurve || []).length === 0
            }
        }
    }

    Component.onCompleted: updateChart()
}
