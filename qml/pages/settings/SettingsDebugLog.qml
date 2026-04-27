// Copyright (c) 2026 The Bitcoin Core developers
// Distributed under the MIT software license, see the accompanying
// file COPYING or http://www.opensource.org/licenses/mit-license.php.

import QtQuick 2.15
import QtQuick.Controls 2.15
import QtQuick.Layouts 1.15
import org.bitcoincore.qt 1.0
import "../../controls"
import "../../components"

Page {
    signal back

    id: root
    objectName: "settingsDebugLog"
    background: null

    property int pendingNewLines: 0
    property int displayedLines: 0
    onPendingNewLinesChanged: if (pendingNewLines > 0) displayedLines = pendingNewLines
    property bool userIsScrolled: false

    Connections {
        target: debugLogModel
        function onNewLinesAdded(count) {
            if (root.userIsScrolled) root.pendingNewLines += count
        }
    }

    // Debounce search text so the C++ filter does not run synchronously on
    // every key press for large log files.
    Timer {
        id: searchDebounce
        interval: 150
        repeat: false
        onTriggered: debugLogModel.filter = searchField.text
    }

    // Periodically refresh the "N min ago" labels in-place.
    Timer {
        interval: 60000
        repeat: true
        running: true
        onTriggered: debugLogModel.updateRelativeTimes()
    }

    header: NavigationBar2 {
        leftItem: NavButton {
            objectName: "debugLogBackButton"
            iconSource: "image://images/caret-left"
            text: qsTr("Back")
            onClicked: root.back()
        }
        centerItem: Header {
            headerBold: true
            headerSize: 18
            header: qsTr("debug.log")
        }
        rightItem: RowLayout {
            spacing: 0
            // Both buttons use a bare Item + centred Icon (not NavButton).
            // Rationale: NavButton's contentItem is a RowLayout that fills
            // the button, placing its 24×24 icon at the left edge — the
            // icon is visually off-centre inside the hover background, and
            // rotating the NavButton swings it in an arc. A centred Icon
            // rotates in place and keeps the visible symbol aligned with
            // the hover box.
            Item {
                id: refreshBtn
                objectName: "debugLogRefreshButton"
                implicitWidth: 52
                implicitHeight: 52

                Rectangle {
                    anchors.fill: parent
                    radius: 5
                    color: refreshArea.containsMouse ? Theme.color.neutral2
                                                     : Theme.color.background
                    Behavior on color { ColorAnimation { duration: 150 } }
                }

                Icon {
                    id: refreshIcon
                    anchors.centerIn: parent
                    source: "image://images/refresh"
                    color: Theme.color.neutral9
                    size: 24

                    RotationAnimation on rotation {
                        id: spinAnimation
                        from: 0
                        to: 360
                        duration: 600
                        running: false
                        easing.type: Easing.InOutQuad
                    }
                }

                MouseArea {
                    id: refreshArea
                    anchors.fill: parent
                    hoverEnabled: true
                    cursorShape: Qt.PointingHandCursor
                    onClicked: {
                        debugLogModel.refresh()
                        spinAnimation.restart()
                    }
                }
            }

            Item {
                id: exportBtn
                objectName: "debugLogExportButton"
                implicitWidth: 52
                implicitHeight: 52

                Rectangle {
                    anchors.fill: parent
                    radius: 5
                    color: exportArea.containsMouse ? Theme.color.neutral2
                                                    : Theme.color.background
                    Behavior on color { ColorAnimation { duration: 150 } }
                }

                Icon {
                    anchors.centerIn: parent
                    source: "image://images/export"
                    color: Theme.color.neutral9
                    size: 24
                }

                MouseArea {
                    id: exportArea
                    anchors.fill: parent
                    hoverEnabled: true
                    cursorShape: Qt.PointingHandCursor
                    onClicked: debugLogModel.openLogFile()
                }
            }
        }
    }

    ColumnLayout {
        anchors.fill: parent
        anchors.margins: 20
        spacing: 10

        TextField {
            id: searchField
            objectName: "debugLogSearchField"
            Layout.fillWidth: true
            implicitHeight: 40
            leftPadding: 40
            rightPadding: 12
            font.family: "Inter"
            font.styleName: "Regular"
            font.pixelSize: 15
            color: Theme.color.neutral9
            placeholderTextColor: Theme.color.neutral5
            placeholderText: qsTr("Search...")
            Accessible.name: qsTr("Search debug log")
            Accessible.role: Accessible.EditableText
            onTextChanged: searchDebounce.restart()
            background: Rectangle {
                color: Theme.color.neutral2
                radius: 10
                Icon {
                    anchors.left: parent.left
                    anchors.leftMargin: 12
                    anchors.verticalCenter: parent.verticalCenter
                    source: "qrc:/icons/search"
                    color: Theme.color.neutral5
                    size: 16
                }
            }
        }

        Item {
            Layout.fillWidth: true
            height: 48

            Rectangle {
                id: newEntriesPill
                anchors.centerIn: parent

                visible: opacity > 0
                opacity: (root.pendingNewLines > 0 && root.userIsScrolled) ? 1.0 : 0.0
                Behavior on opacity { NumberAnimation { duration: 150 } }

                width: 16 + arrowText.implicitWidth + 8 + countText.implicitWidth + 24 + closeText.width + 24
                height: 32
                radius: 16

                Behavior on color { ColorAnimation { duration: 150 } }
                color: pressHandler.pressed ? Theme.color.orangeLight2
                    : hoverHandler.hovered ? Theme.color.orangeLight1
                    : Theme.color.orange

                Text {
                    id: arrowText
                    text: "↑"
                    color: "white"
                    font.pixelSize: 15
                    font.family: "Inter"
                    font.bold: true
                    anchors.left: parent.left
                    anchors.leftMargin: 16
                    anchors.verticalCenter: parent.verticalCenter
                }

                Text {
                    id: countText
                    text: root.displayedLines === 1
                        ? qsTr("1 new entry")
                        : qsTr("%1 new entries").arg(root.displayedLines)
                    color: "white"
                    font.pixelSize: 13
                    font.family: "Inter"
                    anchors.left: arrowText.right
                    anchors.leftMargin: 8
                    anchors.verticalCenter: parent.verticalCenter
                }

                Text {
                    id: closeText
                    text: "×"
                    color: "white"
                    font.pixelSize: 20
                    font.bold: true
                    anchors.right: parent.right
                    anchors.rightMargin: 14
                    anchors.verticalCenter: parent.verticalCenter
                    opacity: closeArea.containsMouse ? 1.0 : 0.85
                }

                MouseArea {
                    id: closeArea
                    anchors {
                        right: parent.right
                        top: parent.top
                        bottom: parent.bottom
                        rightMargin: 6
                    }
                    width: 32
                    hoverEnabled: true
                    cursorShape: Qt.PointingHandCursor
                    onClicked: root.pendingNewLines = 0
                }

                HoverHandler {
                    id: hoverHandler
                    acceptedDevices: PointerDevice.Mouse | PointerDevice.TouchPad
                    cursorShape: Qt.PointingHandCursor
                }

                TapHandler {
                    id: pressHandler
                    acceptedButtons: Qt.LeftButton
                    onTapped: {
                        logView.scrollToTop()
                        root.pendingNewLines = 0
                    }
                }
            }
        }

        MonospaceOutputView {
            id: logView
            objectName: "debugLogListView"
            Layout.fillWidth: true
            Layout.fillHeight: true

            listModel: debugLogModel
            contentRole: "content"
            contentTextFormat: Text.RichText
            leftColumnRole: "lineNumber"
            rightColumnRole: "relativeTime"
            leftColumnSample: "99999"
            rightColumnSample: "99 hr ago"
            fontPixelSize: 11
            contentColor: Theme.color.neutral9
            leftColumnColor: Theme.color.neutral5
            rightColumnColor: Theme.color.neutral5
            accessibleName: qsTr("Debug log entries")
            autoScrollToBottom: false
            // DebugLogModel renders newest-first at the top, so y > 0 means
            // "scrolled away from the newest entries" — which is exactly when
            // the "N new entries" pill should be offered.
            onScrolled: {
                root.userIsScrolled = (y > 0)
                if (logView.atTop && root.pendingNewLines > 0) {
                    root.pendingNewLines = 0
                }
            }
        }

        // Reserved slot so the log view's height does not jitter when the
        // "Load more" affordance appears / disappears. Only the button
        // itself toggles visibility.
        Item {
            Layout.fillWidth: true
            Layout.preferredHeight: 36

            TextButton {
                anchors.centerIn: parent
                text: qsTr("Load more")
                textSize: 13
                bold: false
                visible: debugLogModel.hasMoreLines && logView.atBottom
                onClicked: debugLogModel.loadMore()
            }
        }
    }

    Component.onCompleted: debugLogModel.refresh(true)
}
