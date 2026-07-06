import QtQuick
import QtQuick.Controls
import QtQuick.Layouts

Popup {
    id: wizard
    modal: true
    anchors.centerIn: parent
    width: 600
    height: 500
    closePolicy: Popup.NoAutoClose

    property int currentStep: 0

    ColumnLayout {
        anchors.fill: parent
        anchors.margins: 24
        spacing: 16

        Label {
            text: ["Welcome", "API Configuration", "Ready!"][wizard.currentStep]
            font.pixelSize: 24
            font.bold: true
        }

        RowLayout {
            spacing: 8

            Repeater {
                model: 3

                Rectangle {
                    width: 12
                    height: 12
                    radius: 6
                    color: index <= wizard.currentStep ? "#4dd0e1" : "#444444"
                }
            }
        }

        StackLayout {
            currentIndex: wizard.currentStep
            Layout.fillWidth: true
            Layout.fillHeight: true

            ColumnLayout {
                spacing: 12

                Label {
                    text: "Portfolio Recommender helps you build optimized dividend portfolios."
                    wrapMode: Text.WordWrap
                    Layout.fillWidth: true
                }

                Label {
                    text: "Features:"
                    font.bold: true
                }

                Label {
                    text: "• Yield-targeted portfolio generation\n• Historical back-testing\n• Sensitivity analysis\n• Portfolio import & rebalance"
                    wrapMode: Text.WordWrap
                    Layout.fillWidth: true
                }

                Label {
                    text: "The app works offline with seed data, but for live market data you'll need a free API key."
                    wrapMode: Text.WordWrap
                    opacity: 0.7
                    Layout.fillWidth: true
                }
            }

            ColumnLayout {
                spacing: 12

                Label {
                    text: "Enter your Alpha Vantage API key (free at alphavantage.co):"
                    wrapMode: Text.WordWrap
                    Layout.fillWidth: true
                }

                TextField {
                    id: apiKeyField
                    placeholderText: "e.g., DEMO_KEY"
                    text: settingsController.apiKey
                    Layout.fillWidth: true
                }

                Label {
                    text: "Optional: You can add more providers later in Settings."
                    opacity: 0.6
                }

                Label {
                    text: "Skip this step to use offline seed data only."
                    opacity: 0.6
                    font.italic: true
                }
            }

            ColumnLayout {
                spacing: 12

                Label {
                    text: "✅ You're all set!"
                    font.pixelSize: 18
                }

                Label {
                    text: "Your 20-asset dividend universe is ready.\nGenerate your first portfolio from the sidebar."
                    wrapMode: Text.WordWrap
                    Layout.fillWidth: true
                }
            }
        }

        RowLayout {
            Layout.fillWidth: true

            Button {
                text: "Back"
                visible: wizard.currentStep > 0
                onClicked: wizard.currentStep--
            }

            Item {
                Layout.fillWidth: true
            }

            Button {
                text: wizard.currentStep < 2 ? "Next" : "Get Started"
                highlighted: true
                onClicked: {
                    if (wizard.currentStep === 1 && apiKeyField.text.length > 0) {
                        settingsController.apiKey = apiKeyField.text
                        settingsController.save()
                    }

                    if (wizard.currentStep < 2) {
                        wizard.currentStep++
                    } else {
                        settingsController.setFirstRunComplete()
                        wizard.close()
                    }
                }
            }

            Button {
                text: "Skip"
                visible: wizard.currentStep === 1
                flat: true
                onClicked: wizard.currentStep = 2
            }
        }
    }
}
