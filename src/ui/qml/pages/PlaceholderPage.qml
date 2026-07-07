import QtQuick
import QtQuick.Controls
import QtQuick.Layouts

Page {
    property string pageName: "Page"

    ColumnLayout {
        anchors.centerIn: parent
        spacing: 16

        Label {
            text: pageName
            font.pixelSize: 28
            font.bold: true
            color: "#ffffff"
            Layout.alignment: Qt.AlignHCenter
        }

        Label {
            text: "Coming soon..."
            font.pixelSize: 16
            color: "#808080"
            Layout.alignment: Qt.AlignHCenter
        }
    }
}
