/*
 * SPDX-License-Identifier: GPL-3.0-only
 * MuseScore-CLA-applies
 *
 * MuseScore
 * Music Composition & Notation
 *
 * Copyright (C) 2021 MuseScore BVBA and others
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License version 3 as
 * published by the Free Software Foundation.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <https://www.gnu.org/licenses/>.
 */
import QtQuick 2.15

import MuseScore.Ui 1.0
import MuseScore.UiComponents 1.0

BaseSection {
    id: root

    title: qsTrc("appshell/preferences", "Program start")

    navigation.direction: NavigationPanel.Both

    property alias startupModes: startupModesBox.model
    property var scorePathFilter: null

    property alias panels: panelsView.model

    signal currentStartupModesChanged(int index)
    signal startupScorePathChanged(string path)
    signal panelsVisibleChanged(int panelIndex, bool visible)

    rowSpacing: 16

    RadioButtonGroup {
        id: startupModesBox

        spacing: root.rowSpacing
        orientation: Qt.Vertical

        width: parent.width

        delegate: Row {
            spacing: root.columnSpacing

            RoundedRadioButton {
                anchors.verticalCenter: parent.verticalCenter

                width: root.columnWidth

                checked: modelData.checked
                text: modelData.title

                navigation.name: modelData.title
                navigation.panel: root.navigation
                navigation.row: model.index
                navigation.column: 0

                onToggled: {
                    root.currentStartupModesChanged(model.index)
                }
            }

            FilePicker {
                pathFieldWidth: root.columnWidth
                spacing: root.columnSpacing

                dialogTitle: qsTrc("appshell/preferences", "Choose starting score")
                filter: root.scorePathFilter

                visible: modelData.canSelectScorePath
                path: modelData.scorePath

                navigation: root.navigation
                navigationRowOrderStart: model.index
                navigationColumnOrderStart: 1

                onPathEdited: function(newPath) {
                    root.startupScorePathChanged(newPath)
                }
            }
        }
    }

    ListView {
        id: panelsView

        spacing: root.rowSpacing
        interactive: false

        width: parent.width
        height: contentHeight

        delegate: CheckBox {
            text: modelData.title
            checked: modelData.visible

            navigation.name: modelData.title
            navigation.panel: root.navigation
            navigation.row: startupModesBox.count + model.index
            navigation.column: 0

            onClicked: {
                root.panelsVisibleChanged(model.index, !checked)
            }
        }
    }
}
