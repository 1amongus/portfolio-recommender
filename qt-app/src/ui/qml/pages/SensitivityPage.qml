import QtQuick
import QtQuick.Controls
import QtQuick.Layouts

Page {
    id: page
    property int pageIndex: 2
    property int selectedPointIndex: -1

    function updateInputsFromController() {
        minYieldField.text = Number(sensitivityController.minYield * 100).toFixed(2)
        maxYieldField.text = Number(sensitivityController.maxYield * 100).toFixed(2)
        stepField.text = Number(sensitivityController.step * 100).toFixed(2)
    }

    function pointDetails(index) {
        if (index < 0)
            return ({ holdings: [] })
        return sensitivityController.pointDetails(index)
    }

    function portfolioAtPoint(index) {
        if (index < 0)
            return ({ holdings: [] })
        return sensitivityController.getPortfolioAtPoint(index)
    }

    function loadSelectedPortfolio(index) {
        const portfolio = portfolioAtPoint(index)
        if (!portfolio.holdings || portfolio.holdings.length === 0)
            return

        portfolioController.loadPortfolio(portfolio)
        sensitivityController.setCurrentPortfolio(portfolio.holdings)
        if (StackView.view)
            StackView.view.replace(Qt.resolvedUrl("GeneratePage.qml"))
    }

    Component.onCompleted: {
        updateInputsFromController()
        sensitivityController.setCurrentPortfolio(portfolioController.portfolioResult.holdings || [])
    }

    header: ToolBar {
        contentHeight: 48
        Label {
            anchors.verticalCenter: parent.verticalCenter
            anchors.left: parent.left
            anchors.leftMargin: 16
            text: "Yield & Beta Sensitivity"
            font.pixelSize: 22
            font.bold: true
        }
    }

    Connections {
        target: sensitivityController
        function onCurveDataChanged() {
            if (page.selectedPointIndex >= sensitivityController.curveData.length)
                page.selectedPointIndex = sensitivityController.curveData.length - 1
            chartCanvas.requestPaint()
        }
        function onMinYieldChanged() { updateInputsFromController() }
        function onMaxYieldChanged() { updateInputsFromController() }
        function onStepChanged() { updateInputsFromController() }
        function onCurrentPortfolioChanged() { chartCanvas.requestPaint() }
    }

    Connections {
        target: portfolioController
        function onPortfolioResultChanged() {
            sensitivityController.setCurrentPortfolio(portfolioController.portfolioResult.holdings || [])
        }
    }

    ScrollView {
        anchors.fill: parent
        contentWidth: availableWidth

        ColumnLayout {
            width: parent.width
            spacing: 20
            padding: 24

            Frame {
                Layout.fillWidth: true

                ColumnLayout {
                    anchors.fill: parent
                    spacing: 16

                    RowLayout {
                        Layout.fillWidth: true
                        spacing: 12

                        Label {
                            text: "Min Yield (%)"
                            Layout.alignment: Qt.AlignVCenter
                        }

                        TextField {
                            id: minYieldField
                            Layout.preferredWidth: 120
                            placeholderText: "1.00"
                            validator: DoubleValidator { bottom: 0.0; top: 100.0; decimals: 2 }
                            onEditingFinished: sensitivityController.minYield = Number.fromLocaleString(Qt.locale(), text) / 100.0
                        }

                        Label {
                            text: "Max Yield (%)"
                            Layout.alignment: Qt.AlignVCenter
                        }

                        TextField {
                            id: maxYieldField
                            Layout.preferredWidth: 120
                            placeholderText: "8.00"
                            validator: DoubleValidator { bottom: 0.0; top: 100.0; decimals: 2 }
                            onEditingFinished: sensitivityController.maxYield = Number.fromLocaleString(Qt.locale(), text) / 100.0
                        }

                        Label {
                            text: "Step (%)"
                            Layout.alignment: Qt.AlignVCenter
                        }

                        TextField {
                            id: stepField
                            Layout.preferredWidth: 120
                            placeholderText: "0.25"
                            validator: DoubleValidator { bottom: 0.01; top: 100.0; decimals: 2 }
                            onEditingFinished: sensitivityController.step = Number.fromLocaleString(Qt.locale(), text) / 100.0
                        }

                        Item { Layout.fillWidth: true }

                        Button {
                            text: sensitivityController.isLoading ? "Generating..." : "Generate Curve"
                            enabled: !sensitivityController.isLoading
                            onClicked: {
                                page.selectedPointIndex = -1
                                sensitivityController.minYield = Number.fromLocaleString(Qt.locale(), minYieldField.text) / 100.0
                                sensitivityController.maxYield = Number.fromLocaleString(Qt.locale(), maxYieldField.text) / 100.0
                                sensitivityController.step = Number.fromLocaleString(Qt.locale(), stepField.text) / 100.0
                                sensitivityController.generateCurve()
                            }
                        }
                    }

                    RowLayout {
                        Layout.fillWidth: true
                        spacing: 12

                        ProgressBar {
                            Layout.fillWidth: true
                            from: 0
                            to: 100
                            value: sensitivityController.progress
                            visible: sensitivityController.isLoading || sensitivityController.progress > 0
                        }

                        Label {
                            text: sensitivityController.progress + "%"
                            visible: sensitivityController.isLoading || sensitivityController.progress > 0
                        }
                    }
                }
            }

            Label {
                Layout.fillWidth: true
                visible: sensitivityController.errorMessage.length > 0
                color: "#ff8a80"
                wrapMode: Text.Wrap
                text: sensitivityController.errorMessage
            }

            Frame {
                Layout.fillWidth: true
                Layout.preferredHeight: 440

                Item {
                    id: chartArea
                    anchors.fill: parent
                    anchors.margins: 12
                    property real leftPadding: 56
                    property real rightPadding: 20
                    property real topPadding: 20
                    property real bottomPadding: 42
                    property real minX: sensitivityController.curveData.length > 0 ? sensitivityController.curveData[0].yield : sensitivityController.minYield
                    property real maxX: sensitivityController.curveData.length > 0 ? sensitivityController.curveData[sensitivityController.curveData.length - 1].yield : sensitivityController.maxYield
                    property real minBeta: betaRange.min
                    property real maxBeta: betaRange.max

                    QtObject {
                        id: betaRange
                        readonly property real min: {
                            if (sensitivityController.curveData.length === 0)
                                return 0
                            var minValue = sensitivityController.curveData[0].beta
                            for (var i = 1; i < sensitivityController.curveData.length; ++i)
                                minValue = Math.min(minValue, sensitivityController.curveData[i].beta)
                            return Math.max(0, minValue - 0.1)
                        }

                        readonly property real max: {
                            if (sensitivityController.curveData.length === 0)
                                return 1
                            var maxValue = sensitivityController.curveData[0].beta
                            for (var i = 1; i < sensitivityController.curveData.length; ++i)
                                maxValue = Math.max(maxValue, sensitivityController.curveData[i].beta)
                            return maxValue + 0.1
                        }
                    }

                    function plotWidth() {
                        return Math.max(1, width - leftPadding - rightPadding)
                    }

                    function plotHeight() {
                        return Math.max(1, height - topPadding - bottomPadding)
                    }

                    function xForYield(value) {
                        const span = Math.max(0.0001, maxX - minX)
                        return leftPadding + ((value - minX) / span) * plotWidth()
                    }

                    function yForBeta(value) {
                        const span = Math.max(0.0001, maxBeta - minBeta)
                        return topPadding + (1 - ((value - minBeta) / span)) * plotHeight()
                    }

                    function tooltipText(index) {
                        const details = page.pointDetails(index)
                        const lines = [
                            "Target Yield: " + Number((details.yield || 0) * 100).toFixed(2) + "%",
                            "Achieved Yield: " + Number((details.achievedYield || 0) * 100).toFixed(2) + "%",
                            "Beta: " + Number(details.beta || 0).toFixed(3)
                        ]

                        const holdings = details.holdings || []
                        for (var i = 0; i < holdings.length; ++i) {
                            const holding = holdings[i]
                            lines.push(holding.ticker + ": " + Number(holding.weight * 100).toFixed(1) + "%")
                        }
                        return lines.join("\n")
                    }

                    Canvas {
                        id: chartCanvas
                        anchors.fill: parent

                        onPaint: {
                            const ctx = getContext("2d")
                            ctx.clearRect(0, 0, width, height)
                            ctx.fillStyle = "#1f232b"
                            ctx.fillRect(0, 0, width, height)

                            ctx.strokeStyle = "#5f6b7a"
                            ctx.lineWidth = 1
                            ctx.beginPath()
                            ctx.moveTo(chartArea.leftPadding, chartArea.topPadding)
                            ctx.lineTo(chartArea.leftPadding, height - chartArea.bottomPadding)
                            ctx.lineTo(width - chartArea.rightPadding, height - chartArea.bottomPadding)
                            ctx.stroke()

                            ctx.fillStyle = "#cfd8dc"
                            ctx.font = "12px sans-serif"
                            ctx.fillText("Beta", 8, 18)
                            ctx.fillText("Yield (%)", width - 72, height - 10)

                            if (sensitivityController.curveData.length === 0)
                                return

                            ctx.strokeStyle = "#26a69a"
                            ctx.lineWidth = 2
                            ctx.beginPath()
                            for (var i = 0; i < sensitivityController.curveData.length; ++i) {
                                const point = sensitivityController.curveData[i]
                                const x = chartArea.xForYield(point.yield)
                                const y = chartArea.yForBeta(point.beta)
                                if (i === 0)
                                    ctx.moveTo(x, y)
                                else
                                    ctx.lineTo(x, y)
                            }
                            ctx.stroke()

                            if (sensitivityController.currentPortfolioYield > 0 || sensitivityController.currentPortfolioBeta > 0) {
                                const markerX = chartArea.xForYield(sensitivityController.currentPortfolioYield)
                                const markerY = chartArea.yForBeta(sensitivityController.currentPortfolioBeta)
                                ctx.fillStyle = "#ffd54f"
                                ctx.beginPath()
                                ctx.arc(markerX, markerY, 7, 0, Math.PI * 2)
                                ctx.fill()
                                ctx.strokeStyle = "#0f1115"
                                ctx.lineWidth = 2
                                ctx.stroke()
                                ctx.fillStyle = "#ffd54f"
                                ctx.fillText("Your Portfolio", markerX + 10, markerY - 10)
                            }

                            ctx.fillStyle = "#cfd8dc"
                            for (var tick = 0; tick < 5; ++tick) {
                                const ratio = tick / 4
                                const tickY = chartArea.topPadding + ratio * chartArea.plotHeight()
                                const betaValue = chartArea.maxBeta - ratio * (chartArea.maxBeta - chartArea.minBeta)
                                ctx.fillText(Number(betaValue).toFixed(2), 8, tickY + 4)
                            }

                            const tickCount = Math.min(5, sensitivityController.curveData.length)
                            for (var j = 0; j < tickCount; ++j) {
                                const modelIndex = tickCount === 1 ? 0 : Math.round((sensitivityController.curveData.length - 1) * (j / (tickCount - 1)))
                                const modelPoint = sensitivityController.curveData[modelIndex]
                                const tickX = chartArea.xForYield(modelPoint.yield)
                                ctx.fillText(Number(modelPoint.yield * 100).toFixed(2), tickX - 14, height - 18)
                            }
                        }

                        onWidthChanged: requestPaint()
                        onHeightChanged: requestPaint()
                    }

                    Repeater {
                        model: sensitivityController.curveData
                        delegate: Rectangle {
                            width: 12
                            height: 12
                            radius: 6
                            color: page.selectedPointIndex === index ? "#ffca28" : "#80cbc4"
                            border.color: "#0f1115"
                            border.width: 1
                            x: chartArea.xForYield(modelData.yield) - width / 2
                            y: chartArea.yForBeta(modelData.beta) - height / 2

                            MouseArea {
                                id: pointMouseArea
                                anchors.fill: parent
                                hoverEnabled: true
                                onClicked: {
                                    page.selectedPointIndex = index
                                    const portfolio = page.portfolioAtPoint(index)
                                    if (portfolio.holdings && portfolio.holdings.length > 0) {
                                        portfolioController.loadPortfolio(portfolio)
                                        sensitivityController.setCurrentPortfolio(portfolio.holdings)
                                    }
                                }
                            }

                            ToolTip.visible: pointMouseArea.containsMouse || page.selectedPointIndex === index
                            ToolTip.delay: 100
                            ToolTip.text: chartArea.tooltipText(index)
                        }
                    }

                    Label {
                        anchors.centerIn: parent
                        visible: sensitivityController.curveData.length === 0 && !sensitivityController.isLoading
                        text: "Generate a curve to visualize beta sensitivity across yields."
                        color: "#9aa4b2"
                    }
                }
            }

            Frame {
                Layout.fillWidth: true
                visible: page.selectedPointIndex >= 0 && page.selectedPointIndex < sensitivityController.curveData.length

                ColumnLayout {
                    anchors.fill: parent
                    spacing: 12
                    property var details: page.pointDetails(page.selectedPointIndex)

                    Label {
                        text: "Selected Point"
                        font.bold: true
                        font.pixelSize: 18
                    }

                    RowLayout {
                        Layout.fillWidth: true
                        Label { text: "Target Yield: " + Number((details.yield || 0) * 100).toFixed(2) + "%" }
                        Label { text: "Achieved Yield: " + Number((details.achievedYield || 0) * 100).toFixed(2) + "%" }
                        Label { text: "Beta: " + Number(details.beta || 0).toFixed(3) }
                        Item { Layout.fillWidth: true }
                        Button {
                            text: "Use This Portfolio"
                            onClicked: page.loadSelectedPortfolio(page.selectedPointIndex)
                        }
                    }

                    Repeater {
                        model: details.holdings || []
                        delegate: Rectangle {
                            Layout.fillWidth: true
                            implicitHeight: 44
                            color: index % 2 === 0 ? "#252a33" : "#20242c"

                            RowLayout {
                                anchors.fill: parent
                                anchors.margins: 10
                                Label { text: modelData.ticker; Layout.preferredWidth: 100 }
                                Label { text: "Weight: " + Number(modelData.weight * 100).toFixed(2) + "%"; Layout.preferredWidth: 140 }
                                Label { text: "Yield: " + Number(modelData.yield * 100).toFixed(2) + "%"; Layout.preferredWidth: 140 }
                                Label { text: "Beta: " + Number(modelData.beta).toFixed(3); Layout.preferredWidth: 120 }
                            }
                        }
                    }
                }
            }
        }
    }
}
