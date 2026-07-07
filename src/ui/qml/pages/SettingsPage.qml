import QtQuick
import QtQuick.Controls
import QtQuick.Layouts

Page {
    ScrollView {
        anchors.fill: parent
        contentWidth: availableWidth

        ColumnLayout {
            width: parent.width
            spacing: 20

            Item { Layout.preferredHeight: 24 }

            Label {
                text: "Settings"
                font.pixelSize: 24
                font.bold: true
                color: "#ffffff"
                Layout.leftMargin: 24
            }

            // API Key section
            GroupBox {
                title: "API Keys"
                Layout.fillWidth: true
                Layout.leftMargin: 24
                Layout.rightMargin: 24

                ColumnLayout {
                    anchors.fill: parent
                    spacing: 12

                    RowLayout {
                        spacing: 12
                        Label { text: "Alpha Vantage:"; color: "#b0b0b0" }
                        TextField {
                            id: apiKeyField
                            text: settingsController.alphaVantageApiKey
                            placeholderText: "Enter API key..."
                            echoMode: TextInput.Password
                            Layout.fillWidth: true
                        }
                        Button {
                            text: "Save"
                            onClicked: settingsController.setAlphaVantageApiKey(apiKeyField.text)
                        }
                    }
                }
            }

            // Universe section
            GroupBox {
                title: "Asset Universe"
                Layout.fillWidth: true
                Layout.leftMargin: 24
                Layout.rightMargin: 24

                ColumnLayout {
                    anchors.fill: parent
                    spacing: 12

                    Label {
                        text: "The optimizer uses these assets to build portfolios."
                        color: "#808080"
                        wrapMode: Text.WordWrap
                    }

                    Button {
                        text: "Reset to Default Universe (20 assets)"
                        onClicked: settingsController.resetUniverse()
                    }
                }
            }

            Item { Layout.preferredHeight: 24 }
        }
    }
}
