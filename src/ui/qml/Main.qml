import QtQuick
import QtQuick.Controls
import QtQuick.Layouts

import "pages"

ApplicationWindow {
    id: root
    visible: true
    width: 1280
    height: 800
    title: "Portfolio Recommender"
    color: "#1a1d23"

    Material.theme: Material.Dark
    Material.accent: Material.Teal

    Component.onCompleted: {
        if (assistantController) assistantController.greet()
    }

    RowLayout {
        anchors.fill: parent
        spacing: 0

        // Navigation sidebar
        Rectangle {
            Layout.fillHeight: true
            Layout.preferredWidth: 180
            color: "#14161a"

            ColumnLayout {
                anchors.fill: parent
                anchors.margins: 8
                spacing: 4

                Label {
                    text: "📊 Portfolio\nRecommender"
                    font.pixelSize: 15
                    font.bold: true
                    color: "#e0e0e0"
                    padding: 12
                }

                Repeater {
                    model: ["Generate", "Backtest", "Sensitivity", "Import", "Saved", "Settings"]
                    delegate: Button {
                        Layout.fillWidth: true
                        text: modelData
                        flat: true
                        highlighted: stackView.currentIndex === index
                        onClicked: {
                            stackView.currentIndex = index
                            if (assistantController) assistantController.suggestForPage(modelData)
                        }
                    }
                }

                Item { Layout.fillHeight: true }

                Label {
                    text: "v2.0 • PySide6"
                    font.pixelSize: 10
                    color: "#555"
                    Layout.alignment: Qt.AlignHCenter
                    padding: 8
                }
            }
        }

        // Main content area
        StackLayout {
            id: stackView
            Layout.fillWidth: true
            Layout.fillHeight: true

            GeneratePage {}
            BacktestPage {}
            SensitivityPage {}
            ImportPage {}
            SavedPage {}
            SettingsPage {}
        }

        // AI Assistant panel
        Rectangle {
            Layout.fillHeight: true
            Layout.preferredWidth: 300
            color: "#12141a"
            border.color: "#2a2d35"
            border.width: 1

            ColumnLayout {
                anchors.fill: parent
                anchors.margins: 8
                spacing: 8

                // Header
                RowLayout {
                    Layout.fillWidth: true
                    Label {
                        text: "🤖 AI Assistant"
                        font.pixelSize: 14
                        font.bold: true
                        color: "#e0e0e0"
                        Layout.fillWidth: true
                    }
                    Button {
                        text: "🗑️"
                        flat: true
                        implicitWidth: 32
                        implicitHeight: 32
                        onClicked: { if (assistantController) assistantController.clearHistory() }
                    }
                }

                // Chat history
                ListView {
                    id: chatView
                    Layout.fillWidth: true
                    Layout.fillHeight: true
                    clip: true
                    spacing: 8
                    model: assistantController ? assistantController.history : []

                    delegate: Rectangle {
                        required property var modelData
                        required property int index
                        width: chatView.width - 8
                        height: msgLabel.implicitHeight + 16
                        radius: 8
                        color: modelData.role === "user" ? "#1e3a5f" : "#1e2d1e"

                        Label {
                            id: msgLabel
                            anchors.fill: parent
                            anchors.margins: 8
                            text: modelData.content
                            wrapMode: Text.WordWrap
                            color: "#e0e0e0"
                            font.pixelSize: 12
                        }
                    }

                    onCountChanged: {
                        Qt.callLater(function() { chatView.positionViewAtEnd() })
                    }
                }

                // Thinking indicator
                Label {
                    text: "💭 Thinking..."
                    color: "#808080"
                    font.italic: true
                    visible: assistantController ? assistantController.isThinking : false
                }

                // Input area
                RowLayout {
                    Layout.fillWidth: true
                    spacing: 4

                    TextField {
                        id: chatInput
                        Layout.fillWidth: true
                        placeholderText: "Ask me anything..."
                        font.pixelSize: 12
                        onAccepted: sendBtn.clicked()
                    }

                    Button {
                        id: sendBtn
                        text: "➤"
                        implicitWidth: 36
                        highlighted: true
                        enabled: chatInput.text.trim().length > 0 && assistantController && !assistantController.isThinking
                        onClicked: {
                            if (chatInput.text.trim().length > 0 && assistantController) {
                                assistantController.sendMessage(chatInput.text.trim())
                                chatInput.text = ""
                            }
                        }
                    }
                }
            }
        }
    }
}
