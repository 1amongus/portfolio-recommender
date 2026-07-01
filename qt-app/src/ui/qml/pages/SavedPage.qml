import QtQuick
import QtQuick.Controls

Page {
    property int pageIndex: 4
    title: "Saved"

    Label {
        anchors.centerIn: parent
        text: "Saved\nComing Soon"
        horizontalAlignment: Text.AlignHCenter
        font.pixelSize: 28
    }
}