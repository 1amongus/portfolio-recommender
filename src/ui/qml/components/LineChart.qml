import QtQuick
import QtQuick.Controls

// Simple line chart drawn with Canvas
Canvas {
    id: chart
    property var dataPoints: []  // [{x, y}] - y values to plot
    property string title: ""
    property string xLabel: ""
    property string yLabel: ""
    property color lineColor: "#4ecdc4"
    property color fillColor: "#1a4040"
    property bool showZeroLine: true

    onDataPointsChanged: requestPaint()

    onPaint: {
        var ctx = getContext("2d")
        ctx.reset()

        if (!dataPoints || dataPoints.length < 2) return

        var margin = {top: 30, right: 20, bottom: 30, left: 50}
        var plotW = width - margin.left - margin.right
        var plotH = height - margin.top - margin.bottom

        // Title
        ctx.fillStyle = "#e0e0e0"
        ctx.font = "bold 13px sans-serif"
        ctx.textAlign = "center"
        ctx.fillText(title, width / 2, 16)

        // Find min/max
        var minY = Infinity, maxY = -Infinity
        for (var i = 0; i < dataPoints.length; i++) {
            if (dataPoints[i].y < minY) minY = dataPoints[i].y
            if (dataPoints[i].y > maxY) maxY = dataPoints[i].y
        }
        // Pad range
        var rangeY = maxY - minY || 1
        minY -= rangeY * 0.1
        maxY += rangeY * 0.1

        function mapX(idx) { return margin.left + (idx / (dataPoints.length - 1)) * plotW }
        function mapY(val) { return margin.top + plotH - ((val - minY) / (maxY - minY)) * plotH }

        // Grid lines
        ctx.strokeStyle = "#2a2d35"
        ctx.lineWidth = 1
        for (var g = 0; g <= 4; g++) {
            var gy = margin.top + (g / 4) * plotH
            ctx.beginPath()
            ctx.moveTo(margin.left, gy)
            ctx.lineTo(margin.left + plotW, gy)
            ctx.stroke()
            
            var label = (maxY - (g / 4) * (maxY - minY)).toFixed(1)
            ctx.fillStyle = "#808080"
            ctx.font = "10px sans-serif"
            ctx.textAlign = "right"
            ctx.fillText(label, margin.left - 5, gy + 4)
        }

        // Zero line
        if (showZeroLine && minY < 0 && maxY > 0) {
            var zeroY = mapY(0)
            ctx.strokeStyle = "#555555"
            ctx.setLineDash([4, 4])
            ctx.beginPath()
            ctx.moveTo(margin.left, zeroY)
            ctx.lineTo(margin.left + plotW, zeroY)
            ctx.stroke()
            ctx.setLineDash([])
        }

        // Fill area under line
        ctx.beginPath()
        ctx.moveTo(mapX(0), mapY(0))
        for (var f = 0; f < dataPoints.length; f++) {
            ctx.lineTo(mapX(f), mapY(dataPoints[f].y))
        }
        ctx.lineTo(mapX(dataPoints.length - 1), mapY(0))
        ctx.closePath()
        ctx.fillStyle = fillColor
        ctx.globalAlpha = 0.3
        ctx.fill()
        ctx.globalAlpha = 1.0

        // Line
        ctx.beginPath()
        ctx.moveTo(mapX(0), mapY(dataPoints[0].y))
        for (var p = 1; p < dataPoints.length; p++) {
            ctx.lineTo(mapX(p), mapY(dataPoints[p].y))
        }
        ctx.strokeStyle = lineColor
        ctx.lineWidth = 2.5
        ctx.stroke()

        // Dots
        for (var d = 0; d < dataPoints.length; d++) {
            ctx.beginPath()
            ctx.arc(mapX(d), mapY(dataPoints[d].y), 3, 0, Math.PI * 2)
            ctx.fillStyle = lineColor
            ctx.fill()
        }

        // X-axis labels
        ctx.fillStyle = "#808080"
        ctx.font = "10px sans-serif"
        ctx.textAlign = "center"
        var step = Math.max(1, Math.floor(dataPoints.length / 6))
        for (var xl = 0; xl < dataPoints.length; xl += step) {
            ctx.fillText(dataPoints[xl].x || (xl + 1).toString(), mapX(xl), height - 8)
        }

        // Axis labels
        if (yLabel) {
            ctx.save()
            ctx.translate(12, height / 2)
            ctx.rotate(-Math.PI / 2)
            ctx.fillStyle = "#808080"
            ctx.font = "10px sans-serif"
            ctx.textAlign = "center"
            ctx.fillText(yLabel, 0, 0)
            ctx.restore()
        }
    }
}
