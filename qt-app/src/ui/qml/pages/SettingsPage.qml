import QtQuick
import QtQuick.Controls
import QtQuick.Layouts

Page {
    id: page
    property int pageIndex: 5

    header: ToolBar {
        contentHeight: 48
        Label {
            anchors.verticalCenter: parent.verticalCenter
            anchors.left: parent.left
            anchors.leftMargin: 16
            text: "Settings"
            font.pixelSize: 22
            font.bold: true
        }
    }

    ColumnLayout {
        anchors.fill: parent
        anchors.margins: 24
        spacing: 16

        Label {
            text: "Market Data Provider: " + settingsController.providerName
            font.bold: true
        }

        TextField {
            id: apiKeyField
            Layout.fillWidth: true
            echoMode: TextInput.Password
            placeholderText: "Alpha Vantage API Key"
            text: settingsController.apiKey
            onTextChanged: settingsController.apiKey = text
        }

        RowLayout {
            Button {
                text: "Save"
                onClicked: settingsController.save()
            }
            Button {
                text: "Reload"
                onClicked: settingsController.load()
            }
            Label {
                text: settingsController.statusMessage
                color: "#80cbc4"
            }
        }
    }
}