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
import MuseScore.Palette 1.0

StyledDialogView {
    id: root

    title: qsTrc("palette", "Palette properties")

    contentWidth: 280
    contentHeight: contentColumn.implicitHeight
    margins: 12

    property var properties

    PalettePropertiesModel {
        id: propertiesModel
    }

    Component.onCompleted: {
        propertiesModel.load(properties)
    }

    Column {
        id: contentColumn
        anchors.fill: parent
        spacing: 12

        StyledTextLabel {
            text: qsTrc("palette", "Name")
            font: ui.theme.bodyBoldFont
        }

        TextInputField {
            currentText: propertiesModel.name

            onCurrentTextEdited: function(newTextValue) {
                propertiesModel.name = newTextValue
            }
        }

        SeparatorLine { anchors.margins: -parent.margins }

        StyledTextLabel {
            text: qsTrc("palette", "Cell size")
            font: ui.theme.bodyBoldFont
        }

        Grid {
            id: grid
            width: parent.width

            columns: 2
            spacing: 12

            Repeater {
                id: repeater

                model: [
                    { title: qsTrc("palette", "Width"), value: propertiesModel.cellWidth, incrementStep: 1 },
                    { title: qsTrc("palette", "Height"), value: propertiesModel.cellHeight, incrementStep: 1 },
                    { title: qsTrc("palette", "Element offset"), value: propertiesModel.elementOffset, measureUnit: qsTrc("global", "sp"), incrementStep: 0.1 },
                    { title: qsTrc("palette", "Scale"), value: propertiesModel.scaleFactor, incrementStep: 0.1 }
                ]

                function setValue(index, value) {
                    if (index === 0) {
                        propertiesModel.cellWidth = value
                    } else if (index === 1) {
                        propertiesModel.cellHeight = value
                    } else if (index === 2) {
                        propertiesModel.elementOffset = value
                    } else if (index === 3) {
                        propertiesModel.scaleFactor = value
                    }
                }

                Column {
                    width: (grid.width - grid.spacing * (grid.columns - 1)) / grid.columns

                    spacing: 8

                    StyledTextLabel {
                        text: modelData["title"]
                    }

                    IncrementalPropertyControl {
                        currentValue: modelData["value"]
                        measureUnitsSymbol: Boolean(modelData["measureUnit"]) ? modelData["measureUnit"] : ""
                        step: modelData["incrementStep"]

                        onValueEdited: function(newValue) {
                            repeater.setValue(model.index, newValue)
                        }
                    }
                }
            }
        }

        CheckBox {
            width: parent.width
            text: qsTrc("palette", "Show grid")

            checked: propertiesModel.showGrid

            onClicked: {
                propertiesModel.showGrid = !checked
            }
        }

        Row {
            width: parent.width
            height: childrenRect.height

            spacing: 12

            FlatButton {
                text: qsTrc("global", "Cancel")

                width: (parent.width - parent.spacing) / 2

                onClicked: {
                    propertiesModel.reject()
                    root.hide()
                }
            }

            FlatButton {
                text: qsTrc("global", "OK")

                width: (parent.width - parent.spacing) / 2

                onClicked: {
                    root.hide()
                }
            }
        }
    }
}
