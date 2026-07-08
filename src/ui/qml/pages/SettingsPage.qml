import QtQuick
import QtQuick.Controls
import QtQuick.Layouts
import QtQuick.Dialogs

Page {
    Component.onCompleted: {
        if (assistantController) assistantController.fetchModels()
    }

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

            // ─── API Keys ───────────────────────────────────
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
                            text: settingsController ? settingsController.alphaVantageApiKey : ""
                            placeholderText: "Enter API key..."
                            echoMode: TextInput.Password
                            Layout.fillWidth: true
                        }
                        Button {
                            text: "Save"
                            onClicked: { if (settingsController) settingsController.setAlphaVantageApiKey(apiKeyField.text) }
                        }
                    }
                }
            }

            // ─── SLM Model Selection ────────────────────────
            GroupBox {
                title: "🤖 AI Model (SLM via Ollama)"
                Layout.fillWidth: true
                Layout.leftMargin: 24
                Layout.rightMargin: 24

                ColumnLayout {
                    anchors.fill: parent
                    spacing: 12

                    RowLayout {
                        spacing: 12
                        Label { text: "Ollama URL:"; color: "#b0b0b0" }
                        TextField {
                            id: baseUrlField
                            text: assistantController ? assistantController.baseUrl : "http://localhost:11434"
                            Layout.fillWidth: true
                        }
                        Button {
                            text: "Save"
                            onClicked: {
                                if (assistantController) {
                                    assistantController.setBaseUrl(baseUrlField.text)
                                    assistantController.fetchModels()
                                }
                            }
                        }
                    }

                    RowLayout {
                        spacing: 12
                        Label { text: "Active Model:"; color: "#b0b0b0" }
                        ComboBox {
                            id: modelCombo
                            Layout.fillWidth: true
                            model: {
                                if (!assistantController || !assistantController.availableModels) return []
                                var names = []
                                var models = assistantController.availableModels
                                for (var i = 0; i < models.length; i++) {
                                    names.push(models[i].name + "  (" + models[i].size + ")")
                                }
                                return names
                            }
                            currentIndex: {
                                if (!assistantController || !assistantController.availableModels) return 0
                                var models = assistantController.availableModels
                                for (var i = 0; i < models.length; i++) {
                                    if (models[i].name === assistantController.model) return i
                                }
                                return 0
                            }
                            onActivated: function(index) {
                                if (assistantController && assistantController.availableModels && index < assistantController.availableModels.length) {
                                    assistantController.setModel(assistantController.availableModels[index].name)
                                }
                            }
                        }
                        Button {
                            text: "↻ Refresh"
                            onClicked: { if (assistantController) assistantController.fetchModels() }
                        }
                    }

                    Label {
                        text: "Current: " + (assistantController ? assistantController.model : "none")
                        color: "#4ecdc4"
                        font.pixelSize: 12
                    }
                }
            }

            // ─── SLM Output Settings ────────────────────────
            GroupBox {
                title: "🎛️ Model Output Settings"
                Layout.fillWidth: true
                Layout.leftMargin: 24
                Layout.rightMargin: 24

                GridLayout {
                    anchors.fill: parent
                    columns: 3
                    columnSpacing: 16
                    rowSpacing: 12

                    Label { text: "Temperature:"; color: "#b0b0b0" }
                    Slider {
                        id: tempSlider
                        from: 0.0
                        to: 2.0
                        stepSize: 0.1
                        value: assistantController ? assistantController.temperature : 0.7
                        Layout.fillWidth: true
                        onMoved: { if (assistantController) assistantController.setTemperature(value) }
                    }
                    Label { text: tempSlider.value.toFixed(1); color: "#4ecdc4"; Layout.preferredWidth: 40 }

                    Label { text: "Max Tokens:"; color: "#b0b0b0" }
                    Slider {
                        id: tokensSlider
                        from: 32
                        to: 4096
                        stepSize: 32
                        value: assistantController ? assistantController.maxTokens : 256
                        Layout.fillWidth: true
                        onMoved: { if (assistantController) assistantController.setMaxTokens(Math.round(value)) }
                    }
                    Label { text: Math.round(tokensSlider.value).toString(); color: "#4ecdc4"; Layout.preferredWidth: 40 }

                    Label { text: "Top P:"; color: "#b0b0b0" }
                    Slider {
                        id: topPSlider
                        from: 0.0
                        to: 1.0
                        stepSize: 0.05
                        value: assistantController ? assistantController.topP : 0.9
                        Layout.fillWidth: true
                        onMoved: { if (assistantController) assistantController.setTopP(value) }
                    }
                    Label { text: topPSlider.value.toFixed(2); color: "#4ecdc4"; Layout.preferredWidth: 40 }
                }
            }

            // ─── Harness Files ──────────────────────────────
            GroupBox {
                title: "📄 Harness Files (Context for AI)"
                Layout.fillWidth: true
                Layout.leftMargin: 24
                Layout.rightMargin: 24

                ColumnLayout {
                    anchors.fill: parent
                    spacing: 12

                    Label {
                        text: "Upload .md files to provide additional context to the AI assistant.\nThese files are included in every conversation as system context."
                        color: "#808080"
                        wrapMode: Text.WordWrap
                        Layout.fillWidth: true
                    }

                    RowLayout {
                        spacing: 12
                        Button {
                            text: "📂 Add Harness File (.md)"
                            highlighted: true
                            onClicked: harnessFileDialog.open()
                        }
                        Button {
                            text: "↻ Refresh"
                            onClicked: { if (assistantController) assistantController.refreshHarnessFiles() }
                        }
                        Label {
                            text: (assistantController ? assistantController.harnessFiles.length : 0) + " file(s) loaded"
                            color: "#4ecdc4"
                        }
                    }

                    // File list
                    Repeater {
                        model: assistantController ? assistantController.harnessFiles : []

                        delegate: Rectangle {
                            required property var modelData
                            required property int index
                            Layout.fillWidth: true
                            height: 44
                            radius: 6
                            color: index % 2 === 0 ? "#22252d" : "#1e2128"

                            RowLayout {
                                anchors.fill: parent
                                anchors.margins: 8
                                spacing: 12

                                Label {
                                    text: "📄"
                                    font.pixelSize: 16
                                }
                                Label {
                                    text: modelData.name
                                    color: "#ffffff"
                                    font.bold: true
                                    Layout.fillWidth: true
                                }
                                Label {
                                    text: modelData.size
                                    color: "#808080"
                                    font.pixelSize: 11
                                }
                                Button {
                                    text: "🗑️"
                                    flat: true
                                    implicitWidth: 32
                                    implicitHeight: 32
                                    onClicked: {
                                        if (assistantController) assistantController.removeHarnessFile(modelData.name)
                                    }
                                }
                            }
                        }
                    }

                    Label {
                        text: "No harness files yet. Add .md files to enrich the AI's context."
                        color: "#555"
                        visible: !assistantController || assistantController.harnessFiles.length === 0
                        font.italic: true
                    }
                }
            }

            // ─── Universe Management ────────────────────────
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
                        onClicked: { if (settingsController) settingsController.resetUniverse() }
                    }
                }
            }

            Item { Layout.preferredHeight: 40 }
        }
    }

    FileDialog {
        id: harnessFileDialog
        title: "Select Markdown harness file"
        nameFilters: ["Markdown files (*.md)", "All files (*)"]
        onAccepted: {
            if (assistantController) assistantController.addHarnessFile(selectedFile.toString())
        }
    }
}
