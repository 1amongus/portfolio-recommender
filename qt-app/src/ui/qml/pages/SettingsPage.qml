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
                    spacing: 12

                    Label {
                        text: "API Keys"
                        font.pixelSize: 18
                        font.bold: true
                    }

                    Label {
                        text: "Market Data Provider: " + settingsController.providerName
                        color: "#b0bec5"
                    }

                    TextField {
                        id: alphaKeyField
                        Layout.fillWidth: true
                        echoMode: TextInput.Password
                        placeholderText: "Alpha Vantage API Key"
                        text: settingsController.alphaVantageApiKey
                        onTextChanged: settingsController.alphaVantageApiKey = text
                    }

                    TextField {
                        Layout.fillWidth: true
                        echoMode: TextInput.Password
                        placeholderText: "Financial Modeling Prep API Key"
                        text: settingsController.fmpApiKey
                        onTextChanged: settingsController.fmpApiKey = text
                    }

                    TextField {
                        Layout.fillWidth: true
                        echoMode: TextInput.Password
                        placeholderText: "Polygon API Key"
                        text: settingsController.polygonApiKey
                        onTextChanged: settingsController.polygonApiKey = text
                    }

                    TextField {
                        Layout.fillWidth: true
                        echoMode: TextInput.Password
                        placeholderText: "FRED API Key"
                        text: settingsController.fredApiKey
                        onTextChanged: settingsController.fredApiKey = text
                    }

                    RowLayout {
                        Layout.fillWidth: true

                        Button {
                            text: "Save"
                            onClicked: settingsController.save()
                        }

                        Button {
                            text: "Reload"
                            onClicked: settingsController.load()
                        }

                        Label {
                            Layout.fillWidth: true
                            text: settingsController.hasApiKey ? "At least one API key configured" : "No saved API keys"
                            color: settingsController.hasApiKey ? "#80cbc4" : "#ffcc80"
                        }
                    }
                }
            }

            Frame {
                Layout.fillWidth: true
                Layout.fillHeight: true

                ColumnLayout {
                    anchors.fill: parent
                    spacing: 12

                    Label {
                        text: "Asset Universe (" + settingsController.universeSize + " tickers)"
                        font.pixelSize: 18
                        font.bold: true
                    }

                    ListView {
                        id: universeList
                        Layout.fillWidth: true
                        Layout.preferredHeight: Math.min(contentHeight, 360)
                        clip: true
                        spacing: 4
                        model: settingsController.universeTickers

                        delegate: Rectangle {
                            width: ListView.view.width
                            height: 64
                            radius: 6
                            color: index % 2 === 0 ? "#252a33" : "#20242c"

                            RowLayout {
                                anchors.fill: parent
                                anchors.margins: 12
                                spacing: 12

                                ColumnLayout {
                                    Layout.fillWidth: true
                                    spacing: 2

                                    Label {
                                        text: modelData.ticker + " — " + modelData.name
                                        font.bold: true
                                    }

                                    Label {
                                        text: modelData.sector + " • Yield " + ((modelData.yield || 0) * 100).toFixed(2) + "% • Beta " + Number(modelData.beta || 0).toFixed(2)
                                        color: "#b0bec5"
                                        wrapMode: Text.Wrap
                                    }
                                }

                                Button {
                                    text: "Remove"
                                    enabled: !settingsController.isRefreshing
                                    onClicked: settingsController.removeTicker(modelData.ticker)
                                }
                            }
                        }
                    }

                    RowLayout {
                        Layout.fillWidth: true

                        TextField {
                            id: addTickerField
                            Layout.fillWidth: true
                            placeholderText: "Add ticker (e.g. MAIN)"
                            enabled: !settingsController.isRefreshing
                            onAccepted: {
                                settingsController.addTicker(text)
                                text = ""
                            }
                        }

                        Button {
                            text: "Add"
                            enabled: !settingsController.isRefreshing && addTickerField.text.trim().length > 0
                            onClicked: {
                                settingsController.addTicker(addTickerField.text)
                                addTickerField.text = ""
                            }
                        }
                    }

                    RowLayout {
                        Layout.fillWidth: true

                        Button {
                            text: "Refresh All from API"
                            enabled: settingsController.hasApiKey && !settingsController.isRefreshing
                            onClicked: settingsController.refreshUniverse()
                        }

                        Label {
                            Layout.fillWidth: true
                            text: settingsController.hasApiKey
                                  ? "Saved provider keys available for live refreshes."
                                  : "Save at least one provider key to enable live refreshes."
                            color: settingsController.hasApiKey ? "#80cbc4" : "#ffcc80"
                            wrapMode: Text.Wrap
                        }
                    }

                    ProgressBar {
                        Layout.fillWidth: true
                        from: 0
                        to: 100
                        value: settingsController.refreshProgress
                        visible: settingsController.isRefreshing
                    }
                }
            }

            Frame {
                Layout.fillWidth: true

                ColumnLayout {
                    anchors.fill: parent
                    spacing: 8

                    Label {
                        text: "App Info"
                        font.pixelSize: 18
                        font.bold: true
                    }

                    Label {
                        text: "Version: " + Qt.application.version
                    }

                    Label {
                        Layout.fillWidth: true
                        text: "Data directory: " + settingsController.dataDirectory
                        wrapMode: Text.WrapAnywhere
                        color: "#b0bec5"
                    }
                }
            }

            Label {
                Layout.fillWidth: true
                text: settingsController.statusMessage
                wrapMode: Text.Wrap
                color: settingsController.statusMessage.toLowerCase().indexOf("fail") >= 0
                       || settingsController.statusMessage.toLowerCase().indexOf("unable") >= 0
                       || settingsController.statusMessage.toLowerCase().indexOf("please") >= 0
                       || settingsController.statusMessage.toLowerCase().indexOf("no ") === 0
                       ? "#ff8a80"
                       : "#80cbc4"
            }
        }
    }
}