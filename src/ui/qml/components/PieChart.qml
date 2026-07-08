import QtQuick
import QtQuick.Controls
import QtQuick.Layouts

// Simple pie chart drawn with Canvas
Canvas {
    id: chart
    property var slices: []  // [{label, value, color}]
    property string title: ""

    onSlicesChanged: requestPaint()

    onPaint: {
        var ctx = getContext("2d")
        ctx.reset()
        
        if (!slices || slices.length === 0) return

        var centerX = width / 2
        var centerY = height / 2 + 10
        var radius = Math.min(width, height) / 2 - 30

        // Title
        ctx.fillStyle = "#e0e0e0"
        ctx.font = "bold 13px sans-serif"
        ctx.textAlign = "center"
        ctx.fillText(title, centerX, 18)

        var total = 0
        for (var i = 0; i < slices.length; i++) total += slices[i].value

        if (total <= 0) return

        var startAngle = -Math.PI / 2
        for (var j = 0; j < slices.length; j++) {
            var sliceAngle = (slices[j].value / total) * 2 * Math.PI
            
            ctx.beginPath()
            ctx.moveTo(centerX, centerY)
            ctx.arc(centerX, centerY, radius, startAngle, startAngle + sliceAngle)
            ctx.closePath()
            ctx.fillStyle = slices[j].color || getColor(j)
            ctx.fill()
            ctx.strokeStyle = "#1a1d23"
            ctx.lineWidth = 2
            ctx.stroke()

            // Label
            var midAngle = startAngle + sliceAngle / 2
            var labelRadius = radius * 0.65
            var lx = centerX + Math.cos(midAngle) * labelRadius
            var ly = centerY + Math.sin(midAngle) * labelRadius
            
            if (slices[j].value / total > 0.05) {
                ctx.fillStyle = "#ffffff"
                ctx.font = "11px sans-serif"
                ctx.textAlign = "center"
                ctx.fillText(slices[j].label, lx, ly)
                ctx.fillText((slices[j].value * 100).toFixed(0) + "%", lx, ly + 14)
            }

            startAngle += sliceAngle
        }
    }

    function getColor(idx) {
        var colors = ["#4ecdc4", "#ffe66d", "#ff6b6b", "#a8e6cf", "#dda0dd", "#87ceeb", "#f0e68c", "#98fb98"]
        return colors[idx % colors.length]
    }
}
