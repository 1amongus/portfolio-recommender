import QtQuick
import QtQuick.Controls
import QtQuick.Layouts

Rectangle {
    id: panel
    color: "#1a2332"
    radius: 8

    property string title: "AI Insight"
    property string text: ""
    property bool generating: false

    ColumnLayout {
        anchors.fill: parent
        anchors.margins: 16
        spacing: 8

        RowLayout {
            Layout.fillWidth: true

            Label {
                text: "🤖 " + panel.title
                font.bold: true
                font.pixelSize: 14
            }

            Item { Layout.fillWidth: true }

            BusyIndicator {
                visible: panel.generating
                running: panel.generating
                implicitWidth: 20
                implicitHeight: 20
            }
        }

        Label {
            id: explanationText
            Layout.fillWidth: true
            wrapMode: Text.WordWrap
            font.pixelSize: 13
            opacity: 0.9
            text: panel.text.length > 0
                  ? panel.text
                  : (panel.generating ? "Generating insight..." : "Click 'Explain' to generate an AI insight.")
        }

        Label {
            text: "⚠️ Template-based insight • SLM model not installed"
            font.pixelSize: 10
            opacity: 0.5
            visible: !explanationController.modelAvailable
        }
    }
}
