import QtQuick
import QtQuick.Controls
import QtQuick.Layouts
import QtCharts

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

    function updateComparisonChart() {
        originalSeries.clear()
        rebalancedSeries.clear()

        const original = importController.originalBacktest.equityCurveData || []
        const rebalanced = importController.rebalancedBacktest.equityCurveData || []
        const combined = original.concat(rebalanced)

        if (combined.length === 0) {
            const now = new Date()
            compareXAxis.min = now
            compareXAxis.max = now
            compareYAxis.min = 0
            compareYAxis.max = 1
            return
        }

        var minX = Number(combined[0].x)
        var maxX = Number(combined[0].x)
        var minY = Number(combined[0].y)
        var maxY = minY

        for (var i = 0; i < original.length; ++i) {
            originalSeries.append(Number(original[i].x), Number(original[i].y))
        }

        for (var j = 0; j < rebalanced.length; ++j) {
            rebalancedSeries.append(Number(rebalanced[j].x), Number(rebalanced[j].y))
        }

        for (var k = 0; k < combined.length; ++k) {
            minX = Math.min(minX, Number(combined[k].x))
            maxX = Math.max(maxX, Number(combined[k].x))
            minY = Math.min(minY, Number(combined[k].y))
            maxY = Math.max(maxY, Number(combined[k].y))
        }

        const span = Math.max(Math.abs(maxY - minY), 0.01)
        compareXAxis.min = new Date(minX)
        compareXAxis.max = new Date(maxX)
        compareYAxis.min = minY - span * 0.1
        compareYAxis.max = maxY + span * 0.1
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

    Connections {
        target: importController
        function onOriginalBacktestChanged() { page.updateComparisonChart() }
        function onRebalancedBacktestChanged() { page.updateComparisonChart() }
    }

    ScrollView {
        anchors.fill: parent
        contentWidth: availableWidth

        ColumnLayout {
            width: parent.width
            spacing: 20
            padding: 24

            RowLayout {
                Layout.fillWidth: true
                spacing: 20

                Frame {
                    Layout.fillWidth: true
                    Layout.alignment: Qt.AlignTop

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
                    Layout.alignment: Qt.AlignTop

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

            Frame {
                Layout.fillWidth: true
                visible: importController.rebalancedHoldings.length > 0

                ColumnLayout {
                    anchors.fill: parent
                    spacing: 16

                    RowLayout {
                        Layout.fillWidth: true

                        Label {
                            text: "Comparative Back-Tests"
                            font.bold: true
                            font.pixelSize: 18
                        }

                        Item { Layout.fillWidth: true }

                        ComboBox {
                            id: compareYearsComboBox
                            model: [1, 3, 5, 10]
                            Layout.preferredWidth: 100
                        }

                        Button {
                            text: "Compare Back-Tests"
                            onClicked: importController.runComparativeBacktest(Number(compareYearsComboBox.currentText))
                        }
                    }

                    ChartView {
                        id: comparisonChart
                        Layout.fillWidth: true
                        Layout.preferredHeight: 360
                        antialiasing: true
                        theme: ChartView.ChartThemeDark
                        legend.visible: true
                        visible: (importController.originalBacktest.equityCurveData || []).length > 0

                        DateTimeAxis {
                            id: compareXAxis
                            format: "yyyy-MM"
                            titleText: "Date"
                            tickCount: 6
                        }

                        ValueAxis {
                            id: compareYAxis
                            titleText: "Portfolio Value"
                            labelFormat: "%.2f"
                        }

                        LineSeries {
                            id: originalSeries
                            name: "Original"
                            axisX: compareXAxis
                            axisY: compareYAxis
                            color: "#888888"
                            width: 2
                        }

                        LineSeries {
                            id: rebalancedSeries
                            name: "Rebalanced"
                            axisX: compareXAxis
                            axisY: compareYAxis
                            color: "#4dd0e1"
                            width: 2
                        }
                    }

                    Frame {
                        Layout.fillWidth: true
                        visible: (importController.originalBacktest.equityCurveData || []).length > 0

                        GridLayout {
                            anchors.fill: parent
                            columns: 3
                            rowSpacing: 10
                            columnSpacing: 18

                            Label { text: "" }
                            Label { text: "Original"; font.bold: true }
                            Label { text: "Rebalanced"; font.bold: true }

                            Label { text: "Total Return"; font.bold: true }
                            Label { text: page.formatPercent(importController.originalBacktest.metrics ? importController.originalBacktest.metrics.totalReturn : 0) }
                            Label { text: page.formatPercent(importController.rebalancedBacktest.metrics ? importController.rebalancedBacktest.metrics.totalReturn : 0) }

                            Label { text: "Sharpe"; font.bold: true }
                            Label { text: Number(importController.originalBacktest.metrics ? importController.originalBacktest.metrics.sharpeRatio : 0).toFixed(2) }
                            Label { text: Number(importController.rebalancedBacktest.metrics ? importController.rebalancedBacktest.metrics.sharpeRatio : 0).toFixed(2) }

                            Label { text: "Max Drawdown"; font.bold: true }
                            Label { text: page.formatPercent(importController.originalBacktest.metrics ? importController.originalBacktest.metrics.maxDrawdown : 0) }
                            Label { text: page.formatPercent(importController.rebalancedBacktest.metrics ? importController.rebalancedBacktest.metrics.maxDrawdown : 0) }
                        }
                    }
                }
            }
        }
    }

    Component.onCompleted: updateComparisonChart()
}
