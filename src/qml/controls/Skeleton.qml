// Copyright (c) 2025 The Bitcoin Core developers
// Distributed under the MIT software license, see the accompanying
// file COPYING or http://www.opensource.org/licenses/mit-license.php.

import QtQuick 2.15

Rectangle {
    id: root
    color: Theme.color.neutral9
    radius: 5

    SequentialAnimation on opacity {
        loops: Animation.Infinite
        running: root.visible
        NumberAnimation { from: 0.4; to: 0.6; duration: 1000; easing.type: Easing.InOutQuad }
        NumberAnimation { from: 0.6; to: 0.4; duration: 1000; easing.type: Easing.InOutQuad }
    }
}
