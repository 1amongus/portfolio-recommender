import QtQuick
import QtQuick.Controls
import QtQuick.Layouts

Pane {
    id: root
    property int currentIndex: 0
    signal pageRequested(int index)

    function pageComponent(index) {
        switch (index) {
        case 0: return Qt.resolvedUrl("../pages/GeneratePage.qml")
        case 1: return Qt.resolvedUrl("../pages/BackTestPage.qml")
        case 2: return Qt.resolvedUrl("../pages/SensitivityPage.qml")
        case 3: return Qt.resolvedUrl("../pages/ImportPage.qml")
        case 4: return Qt.resolvedUrl("../pages/SavedPage.qml")
        case 5: return Qt.resolvedUrl("../pages/SettingsPage.qml")
        default: return Qt.resolvedUrl("../pages/GeneratePage.qml")
        }
    }

    background: Rectangle {
        color: "#1c1f26"
    }

    ListModel {
        id: navModel
        ListElement { title: "Generate"; iconText: "" }
        ListElement { title: "Back-Test"; iconText: "" }
        ListElement { title: "Sensitivity"; iconText: "" }
        ListElement { title: "Import"; iconText: "" }
        ListElement { title: "Saved"; iconText: "" }
        ListElement { title: "Settings"; iconText: "" }
    }

    ColumnLayout {
        anchors.fill: parent
        anchors.margins: 16
        spacing: 12

        Label {
            text: "Portfolio\nRecommender"
            font.pixelSize: 24
            font.bold: true
            Layout.fillWidth: true
        }

        Repeater {
            model: navModel
            delegate: Button {
                required property int index
                required property string title
                required property string iconText
                Layout.fillWidth: true
                text: iconText + "  " + title
                highlighted: root.currentIndex === index
                onClicked: root.pageRequested(index)
            }
        }

        Item { Layout.fillHeight: true }
    }
}