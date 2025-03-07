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
#include "exportdialogmodel.h"

#include <QItemSelectionModel>

#include "async/async.h"
#include "translation.h"
#include "log.h"

using namespace mu::project;
using namespace mu::notation;
using namespace mu::iex::musicxml;

using UnitType = INotationWriter::UnitType;

static const UnitType DEFAULT_EXPORT_UNITTYPE = UnitType::PER_PART;

ExportDialogModel::ExportDialogModel(QObject* parent)
    : QAbstractListModel(parent)
    , m_selectionModel(new QItemSelectionModel(this))
    , m_selectedUnitType(DEFAULT_EXPORT_UNITTYPE)
{
    connect(m_selectionModel, &QItemSelectionModel::selectionChanged, this, &ExportDialogModel::selectionChanged);

    ExportTypeList musicXmlTypes {
        ExportType::makeWithSuffixes({ "mxl" },
                                     qtrc("project/export", "Compressed") + " (*.mxl)",
                                     qtrc("project/export", "Compressed MusicXML files"),
                                     "MusicXmlSettingsPage.qml"),
        ExportType::makeWithSuffixes({ "musicxml" },
                                     qtrc("project/export", "Uncompressed") + " (*.musicxml)",
                                     qtrc("project/export", "Uncompressed MusicXML files"),
                                     "MusicXmlSettingsPage.qml"),
        ExportType::makeWithSuffixes({ "xml" },
                                     qtrc("project/export", "Uncompressed (outdated)") + " (*.xml)",
                                     qtrc("project/export", "Uncompressed MusicXML files"),
                                     "MusicXmlSettingsPage.qml"),
    };

    m_exportTypeList = {
        ExportType::makeWithSuffixes({ "pdf" },
                                     qtrc("project/export", "PDF file"),
                                     qtrc("project/export", "PDF files"),
                                     "PdfSettingsPage.qml"),
        ExportType::makeWithSuffixes({ "png" },
                                     qtrc("project/export", "PNG images"),
                                     qtrc("project/export", "PNG images"),
                                     "PngSettingsPage.qml"),
        ExportType::makeWithSuffixes({ "svg" },
                                     qtrc("project/export", "SVG images"),
                                     qtrc("project/export", "SVG images"),
                                     "SvgSettingsPage.qml"),
        ExportType::makeWithSuffixes({ "mp3" },
                                     qtrc("project/export", "MP3 audio"),
                                     qtrc("project/export", "MP3 audio files"),
                                     "Mp3SettingsPage.qml"),
        ExportType::makeWithSuffixes({ "wav" },
                                     qtrc("project/export", "WAV audio"),
                                     qtrc("project/export", "WAV audio files"),
                                     "AudioSettingsPage.qml"),
        ExportType::makeWithSuffixes({ "ogg" },
                                     qtrc("project/export", "OGG audio"),
                                     qtrc("project/export", "OGG audio files"),
                                     "AudioSettingsPage.qml"),
        ExportType::makeWithSuffixes({ "flac" },
                                     qtrc("project/export", "FLAC audio"),
                                     qtrc("project/export", "FLAC audio files"),
                                     "AudioSettingsPage.qml"),
        ExportType::makeWithSuffixes({ "mid", "midi", "kar" },
                                     qtrc("project/export", "MIDI file"),
                                     qtrc("project/export", "MIDI files"),
                                     "MidiSettingsPage.qml"),
        ExportType::makeWithSubtypes(musicXmlTypes,
                                     qtrc("project/export", "MusicXML")),
        ExportType::makeWithSuffixes({ "brf" },
                                     qtrc("project/export", "Braille"),
                                     qtrc("project/export", "Braille files"))
    };

    m_selectedExportType = m_exportTypeList.front();
}

ExportDialogModel::~ExportDialogModel()
{
    m_selectionModel->deleteLater();
}

void ExportDialogModel::load()
{
    TRACEFUNC;

    beginResetModel();

    IMasterNotationPtr masterNotation = this->masterNotation();
    if (!masterNotation) {
        endResetModel();
        return;
    }

    m_notations << masterNotation->notation();

    ExcerptNotationList excerpts = masterNotation->excerpts().val;
    ExcerptNotationList potentialExcerpts = masterNotation->potentialExcerpts();
    excerpts.insert(excerpts.end(), potentialExcerpts.begin(), potentialExcerpts.end());

    masterNotation->sortExcerpts(excerpts);

    for (IExcerptNotationPtr excerpt : excerpts) {
        m_notations << excerpt->notation();
    }

    endResetModel();

    selectCurrentNotation();
}

QVariant ExportDialogModel::data(const QModelIndex& index, int role) const
{
    if (!index.isValid()) {
        return QVariant();
    }

    INotationPtr notation = m_notations[index.row()];

    switch (role) {
    case RoleTitle:
        return notation->name();
    case RoleIsSelected:
        return m_selectionModel->isSelected(index);
    case RoleIsMain:
        return isMainNotation(notation);
    }

    return QVariant();
}

int ExportDialogModel::rowCount(const QModelIndex&) const
{
    return m_notations.size();
}

QHash<int, QByteArray> ExportDialogModel::roleNames() const
{
    static const QHash<int, QByteArray> roles {
        { RoleTitle, "title" },
        { RoleIsSelected, "isSelected" },
        { RoleIsMain, "isMain" }
    };

    return roles;
}

void ExportDialogModel::setSelected(int scoreIndex, bool selected)
{
    if (!isIndexValid(scoreIndex)) {
        return;
    }

    QModelIndex modelIndex = index(scoreIndex);
    m_selectionModel->select(modelIndex, selected ? QItemSelectionModel::Select : QItemSelectionModel::Deselect);

    emit dataChanged(modelIndex, modelIndex, { RoleIsSelected });
}

void ExportDialogModel::setAllSelected(bool selected)
{
    for (int i = 0; i < rowCount(); i++) {
        setSelected(i, selected);
    }
}

void ExportDialogModel::selectCurrentNotation()
{
    for (int i = 0; i < rowCount(); i++) {
        setSelected(i, m_notations[i] == context()->currentNotation());
    }
}

IMasterNotationPtr ExportDialogModel::masterNotation() const
{
    return context()->currentMasterNotation();
}

bool ExportDialogModel::isMainNotation(INotationPtr notation) const
{
    return masterNotation() && masterNotation()->notation() == notation;
}

bool ExportDialogModel::isIndexValid(int index) const
{
    return index >= 0 && index < m_notations.size();
}

int ExportDialogModel::selectionLength() const
{
    return m_selectionModel->selectedIndexes().size();
}

QVariantList ExportDialogModel::exportTypeList() const
{
    return m_exportTypeList.toVariantList();
}

QVariantMap ExportDialogModel::selectedExportType() const
{
    return m_selectedExportType.toMap();
}

void ExportDialogModel::setExportType(const ExportType& type)
{
    if (m_selectedExportType == type) {
        return;
    }

    m_selectedExportType = type;
    emit selectedExportTypeChanged(type.toMap());

    std::vector<UnitType> unitTypes = exportProjectScenario()->supportedUnitTypes(type);

    IF_ASSERT_FAILED(!unitTypes.empty()) {
        return;
    }

    if (std::find(unitTypes.cbegin(), unitTypes.cend(), m_selectedUnitType) != unitTypes.cend()) {
        return;
    }

    //! NOTE if the writer for the newly selected type doesn't support the currently
    //! selected unit type, select the first supported unit type
    setUnitType(unitTypes.front());
}

void ExportDialogModel::selectExportTypeById(const QString& id)
{
    for (const ExportType& type : m_exportTypeList) {
        // First, check if it's a subtype
        if (type.subtypes.contains(id)) {
            setExportType(type.subtypes.getById(id));
            return;
        }

        if (type.id == id) {
            setExportType(type);
            return;
        }
    }

    LOGW() << "Export type id not found: " << id;
    setExportType(m_exportTypeList.front());
}

QVariantList ExportDialogModel::availableUnitTypes() const
{
    QMap<UnitType, QString> unitTypeNames {
        { UnitType::PER_PAGE, qtrc("project/export", "Each page to a separate file") },
        { UnitType::PER_PART, qtrc("project/export", "Each part to a separate file") },
        { UnitType::MULTI_PART, qtrc("project/export", "All parts combined in one file") },
    };

    QVariantList result;

    for (UnitType type : exportProjectScenario()->supportedUnitTypes(m_selectedExportType)) {
        QVariantMap obj;
        obj["text"] = unitTypeNames[type];
        obj["value"] = static_cast<int>(type);
        result << obj;
    }

    return result;
}

int ExportDialogModel::selectedUnitType() const
{
    return static_cast<int>(m_selectedUnitType);
}

void ExportDialogModel::setUnitType(int unitType)
{
    setUnitType(static_cast<UnitType>(unitType));
}

void ExportDialogModel::setUnitType(UnitType unitType)
{
    if (m_selectedUnitType == unitType) {
        return;
    }

    m_selectedUnitType = unitType;
    emit selectedUnitTypeChanged(unitType);
}

bool ExportDialogModel::exportScores()
{
    INotationPtrList notations;

    for (const QModelIndex& index : m_selectionModel->selectedIndexes()) {
        notations.push_back(m_notations[index.row()]);
    }

    if (notations.empty()) {
        return false;
    }

    ExcerptNotationList excerptsToInit;
    ExcerptNotationList potentialExcerpts = masterNotation()->potentialExcerpts();

    for (const INotationPtr& notation : notations) {
        auto it = std::find_if(potentialExcerpts.cbegin(), potentialExcerpts.cend(), [notation](const IExcerptNotationPtr& excerpt) {
            return excerpt->notation() == notation;
        });

        if (it != potentialExcerpts.cend()) {
            excerptsToInit.push_back(*it);
        }
    }

    masterNotation()->initExcerpts(excerptsToInit);

    RetVal<io::path_t> exportPath = exportProjectScenario()->askExportPath(notations, m_selectedExportType, m_selectedUnitType);
    if (!exportPath.ret) {
        return false;
    }

    QMetaObject::invokeMethod(qApp, [this, notations, exportPath]() {
        exportProjectScenario()->exportScores(notations, exportPath.val, m_selectedUnitType,
                                              shouldDestinationFolderBeOpenedOnExport());
    }, Qt::QueuedConnection);

    return true;
}

int ExportDialogModel::pdfResolution() const
{
    return imageExportConfiguration()->exportPdfDpiResolution();
}

void ExportDialogModel::setPdfResolution(const int& resolution)
{
    if (resolution == pdfResolution()) {
        return;
    }

    imageExportConfiguration()->setExportPdfDpiResolution(resolution);
    emit pdfResolutionChanged(resolution);
}

int ExportDialogModel::pngResolution() const
{
    return imageExportConfiguration()->exportPngDpiResolution();
}

void ExportDialogModel::setPngResolution(const int& resolution)
{
    if (resolution == pngResolution()) {
        return;
    }

    imageExportConfiguration()->setExportPngDpiResolution(resolution);
    emit pngResolutionChanged(resolution);
}

bool ExportDialogModel::pngTransparentBackground() const
{
    return imageExportConfiguration()->exportPngWithTransparentBackground();
}

void ExportDialogModel::setPngTransparentBackground(const bool& transparent)
{
    if (transparent == pngTransparentBackground()) {
        return;
    }

    imageExportConfiguration()->setExportPngWithTransparentBackground(transparent);
    emit pngTransparentBackgroundChanged(transparent);
}

QList<int> ExportDialogModel::availableSampleRates() const
{
    const std::vector<int>& rates = audioExportConfiguration()->availableSampleRates();
    return QList<int>(rates.cbegin(), rates.cend());
}

int ExportDialogModel::sampleRate() const
{
    return audioExportConfiguration()->exportSampleRate();
}

void ExportDialogModel::setSampleRate(int rate)
{
    if (rate == sampleRate()) {
        return;
    }

    audioExportConfiguration()->setExportSampleRate(rate);
    emit sampleRateChanged(rate);
}

QList<int> ExportDialogModel::availableBitRates() const
{
    const std::vector<int>& rates = audioExportConfiguration()->availableMp3BitRates();
    return QList<int>(rates.cbegin(), rates.cend());
}

int ExportDialogModel::bitRate() const
{
    return audioExportConfiguration()->exportMp3Bitrate();
}

void ExportDialogModel::setBitRate(int rate)
{
    if (rate == bitRate()) {
        return;
    }

    audioExportConfiguration()->setExportMp3Bitrate(rate);
    emit bitRateChanged(rate);
}

bool ExportDialogModel::midiExpandRepeats() const
{
    NOT_IMPLEMENTED;
    return true;
}

void ExportDialogModel::setMidiExpandRepeats(bool expandRepeats)
{
    if (expandRepeats == midiExpandRepeats()) {
        return;
    }

    NOT_IMPLEMENTED;
    emit midiExpandRepeatsChanged(expandRepeats);
}

bool ExportDialogModel::midiExportRpns() const
{
    return midiImportExportConfiguration()->isMidiExportRpns();
}

void ExportDialogModel::setMidiExportRpns(bool exportRpns)
{
    if (exportRpns == midiExportRpns()) {
        return;
    }

    midiImportExportConfiguration()->setIsMidiExportRpns(exportRpns);
    emit midiExportRpnsChanged(exportRpns);
}

QVariantList ExportDialogModel::musicXmlLayoutTypes() const
{
    QMap<MusicXmlLayoutType, QString> musicXmlLayoutTypeNames {
        //: Specifies to which extent layout customizations should be exported to MusicXML.
        { MusicXmlLayoutType::AllLayout, qtrc("project/export", "All layout") },
        //: Specifies to which extent layout customizations should be exported to MusicXML.
        { MusicXmlLayoutType::AllBreaks, qtrc("project/export", "System and page breaks") },
        //: Specifies to which extent layout customizations should be exported to MusicXML.
        { MusicXmlLayoutType::ManualBreaks, qtrc("project/export", "Manually added system and page breaks only") },
        //: Specifies to which extent layout customizations should be exported to MusicXML.
        { MusicXmlLayoutType::None, qtrc("project/export", "No system or page breaks") },
    };

    QVariantList result;

    for (MusicXmlLayoutType type : musicXmlLayoutTypeNames.keys()) {
        QVariantMap obj;
        obj["text"] = musicXmlLayoutTypeNames[type];
        obj["value"] = static_cast<int>(type);
        result << obj;
    }

    return result;
}

ExportDialogModel::MusicXmlLayoutType ExportDialogModel::musicXmlLayoutType() const
{
    if (musicXmlConfiguration()->musicxmlExportLayout()) {
        return MusicXmlLayoutType::AllLayout;
    }
    switch (musicXmlConfiguration()->musicxmlExportBreaksType()) {
    case IMusicXmlConfiguration::MusicxmlExportBreaksType::All:
        return MusicXmlLayoutType::AllBreaks;
    case IMusicXmlConfiguration::MusicxmlExportBreaksType::Manual:
        return MusicXmlLayoutType::ManualBreaks;
    case IMusicXmlConfiguration::MusicxmlExportBreaksType::No:
        return MusicXmlLayoutType::None;
    }

    return MusicXmlLayoutType::AllLayout;
}

void ExportDialogModel::setMusicXmlLayoutType(MusicXmlLayoutType layoutType)
{
    if (layoutType == musicXmlLayoutType()) {
        return;
    }
    switch (layoutType) {
    case MusicXmlLayoutType::AllLayout:
        musicXmlConfiguration()->setMusicxmlExportLayout(true);
        break;
    case MusicXmlLayoutType::AllBreaks:
        musicXmlConfiguration()->setMusicxmlExportLayout(false);
        musicXmlConfiguration()->setMusicxmlExportBreaksType(IMusicXmlConfiguration::MusicxmlExportBreaksType::All);
        break;
    case MusicXmlLayoutType::ManualBreaks:
        musicXmlConfiguration()->setMusicxmlExportLayout(false);
        musicXmlConfiguration()->setMusicxmlExportBreaksType(IMusicXmlConfiguration::MusicxmlExportBreaksType::Manual);
        break;
    case MusicXmlLayoutType::None:
        musicXmlConfiguration()->setMusicxmlExportLayout(false);
        musicXmlConfiguration()->setMusicxmlExportBreaksType(IMusicXmlConfiguration::MusicxmlExportBreaksType::No);
        break;
    }
    emit musicXmlLayoutTypeChanged(layoutType);
}

bool ExportDialogModel::shouldDestinationFolderBeOpenedOnExport() const
{
    return configuration()->shouldDestinationFolderBeOpenedOnExport();
}

void ExportDialogModel::setShouldDestinationFolderBeOpenedOnExport(bool enabled)
{
    if (enabled == shouldDestinationFolderBeOpenedOnExport()) {
        return;
    }

    configuration()->setShouldDestinationFolderBeOpenedOnExport(enabled);
    emit shouldDestinationFolderBeOpenedOnExportChanged(enabled);
}
