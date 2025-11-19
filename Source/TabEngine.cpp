#include "TabEngine.h"

TabEngine::TabEngine()
    : numStrings(6), rootNote("E"), tuningType(TuningType::Standard), currentSectionIndex(0), currentPartIndex(0)
{
    // Initialize custom tuning with standard E
    customTuning = GuitarTuning::createStandard("E", 6);

    // Start with one default section
    sections.push_back(TabSection("Intro", numStrings, 16));
}

TabEngine::~TabEngine()
{
}

void TabEngine::setNumStrings(int num)
{
    if (num < 4 || num > 9)
        return;

    numStrings = num;

    // Resize all columns in all parts in all sections
    for (auto& section : sections)
    {
        for (auto& part : section.parts)
        {
            for (auto& col : part.columns)
                col = TabColumn(numStrings);
        }
    }

    // Update tuning for new string count
    updateTuning();

    notifyListeners();
}

void TabEngine::setRootNote(const juce::String& note)
{
    rootNote = note;
    updateTuning();
    notifyListeners();
}

void TabEngine::setTuningType(TuningType type)
{
    tuningType = type;
    updateTuning();
    notifyListeners();
}

void TabEngine::setCustomStringNote(int stringIndex, const juce::String& note)
{
    if (stringIndex >= 0 && stringIndex < (int)customTuning.notes.size())
    {
        customTuning.notes[stringIndex] = note;
        notifyListeners();
    }
}

juce::String TabEngine::getStringNote(int stringIndex) const
{
    auto tuning = getCurrentTuning();
    if (stringIndex >= 0 && stringIndex < (int)tuning.notes.size())
        return tuning.notes[stringIndex];
    return "";
}

void TabEngine::updateTuning()
{
    // Update custom tuning based on current settings
    switch (tuningType)
    {
        case TuningType::Standard:
            customTuning = GuitarTuning::createStandard(rootNote, numStrings);
            break;

        case TuningType::Drop:
            customTuning = GuitarTuning::createDrop(rootNote, numStrings);
            break;

        case TuningType::Open:
            customTuning = GuitarTuning::createOpen(rootNote, numStrings);
            break;

        case TuningType::Custom:
            // Resize if needed, but keep existing notes
            while ((int)customTuning.notes.size() < numStrings)
                customTuning.notes.push_back("E");
            while ((int)customTuning.notes.size() > numStrings)
                customTuning.notes.pop_back();
            break;
    }
}

GuitarTuning TabEngine::getCurrentTuning() const
{
    return customTuning;
}

// Section management
void TabEngine::addSection(const juce::String& name)
{
    sections.push_back(TabSection(name, numStrings, 16));
    notifyListeners();
}

void TabEngine::removeSection(int sectionIndex)
{
    if (sectionIndex >= 0 && sectionIndex < (int)sections.size() && sections.size() > 1)
    {
        sections.erase(sections.begin() + sectionIndex);

        // Adjust current section index if needed
        if (currentSectionIndex >= (int)sections.size())
            currentSectionIndex = (int)sections.size() - 1;
        if (currentSectionIndex < 0)
            currentSectionIndex = 0;

        notifyListeners();
    }
}

void TabEngine::renameSection(int sectionIndex, const juce::String& newName)
{
    if (sectionIndex >= 0 && sectionIndex < (int)sections.size())
    {
        sections[sectionIndex].name = newName;
        notifyListeners();
    }
}

juce::String TabEngine::getSectionName(int sectionIndex) const
{
    if (sectionIndex >= 0 && sectionIndex < (int)sections.size())
        return sections[sectionIndex].name;
    return "";
}

void TabEngine::setCurrentSection(int sectionIndex)
{
    if (sectionIndex >= 0 && sectionIndex < (int)sections.size())
    {
        currentSectionIndex = sectionIndex;
        currentPartIndex = 0; // Reset to first part when switching sections
        notifyListeners();
    }
}

void TabEngine::clearSection(int sectionIndex)
{
    if (sectionIndex >= 0 && sectionIndex < (int)sections.size())
    {
        // Clear all parts in the section by recreating them with empty data
        sections[sectionIndex].parts.clear();
        sections[sectionIndex].parts.push_back(TabPart("Part 1", numStrings, 16));

        // Reset current part index if this is the current section
        if (sectionIndex == currentSectionIndex)
            currentPartIndex = 0;

        notifyListeners();
    }
}

std::unique_ptr<juce::XmlElement> TabEngine::copySectionToXML(int sectionIndex) const
{
    if (sectionIndex < 0 || sectionIndex >= (int)sections.size())
        return nullptr;

    auto xml = std::make_unique<juce::XmlElement>("SectionClipboard");
    xml->setAttribute("name", sections[sectionIndex].name);

    // Save all parts in this section
    for (int p = 0; p < (int)sections[sectionIndex].parts.size(); ++p)
    {
        auto* partXml = xml->createNewChildElement("Part");
        partXml->setAttribute("index", p);
        partXml->setAttribute("name", sections[sectionIndex].parts[p].name);
        partXml->setAttribute("numColumns", (int)sections[sectionIndex].parts[p].columns.size());

        for (int col = 0; col < (int)sections[sectionIndex].parts[p].columns.size(); ++col)
        {
            auto* columnXml = partXml->createNewChildElement("Column");
            columnXml->setAttribute("index", col);

            for (int str = 0; str < numStrings; ++str)
            {
                int fret = sections[sectionIndex].parts[p].columns[col].getFret(str);
                if (fret >= 0)
                {
                    auto* noteXml = columnXml->createNewChildElement("Note");
                    noteXml->setAttribute("string", str);
                    noteXml->setAttribute("fret", fret);
                    noteXml->setAttribute("technique", (int)sections[sectionIndex].parts[p].columns[col].getTechnique(str));
                }
            }
        }
    }

    return xml;
}

void TabEngine::pasteSectionFromXML(int sectionIndex, const juce::XmlElement& xml)
{
    if (sectionIndex < 0 || sectionIndex >= (int)sections.size())
        return;

    if (!xml.hasTagName("SectionClipboard"))
        return;

    // Clear existing parts in target section
    sections[sectionIndex].parts.clear();

    // Load parts from clipboard
    for (auto* partXml : xml.getChildIterator())
    {
        if (partXml->hasTagName("Part"))
        {
            juce::String partName = partXml->getStringAttribute("name", "Part 1");
            int numCols = partXml->getIntAttribute("numColumns", 16);

            sections[sectionIndex].parts.push_back(TabPart(partName, numStrings, numCols));
            int pIndex = (int)sections[sectionIndex].parts.size() - 1;

            for (auto* columnXml : partXml->getChildIterator())
            {
                if (columnXml->hasTagName("Column"))
                {
                    int colIndex = columnXml->getIntAttribute("index");

                    if (colIndex >= 0 && colIndex < numCols)
                    {
                        for (auto* noteXml : columnXml->getChildIterator())
                        {
                            if (noteXml->hasTagName("Note"))
                            {
                                int str = noteXml->getIntAttribute("string");
                                int fret = noteXml->getIntAttribute("fret");
                                int tech = noteXml->getIntAttribute("technique", (int)Technique::None);
                                sections[sectionIndex].parts[pIndex].columns[colIndex].setFret(str, fret);
                                sections[sectionIndex].parts[pIndex].columns[colIndex].setTechnique(str, (Technique)tech);
                            }
                        }
                    }
                }
            }
        }
    }

    // Ensure section has at least one part
    if (sections[sectionIndex].parts.empty())
        sections[sectionIndex].parts.push_back(TabPart("Part 1", numStrings, 16));

    // Reset current part index if this is the current section
    if (sectionIndex == currentSectionIndex)
    {
        if (currentPartIndex >= (int)sections[sectionIndex].parts.size())
            currentPartIndex = 0;
    }

    notifyListeners();
}

TabSection* TabEngine::getCurrentSectionPtr()
{
    if (currentSectionIndex >= 0 && currentSectionIndex < (int)sections.size())
        return &sections[currentSectionIndex];
    return nullptr;
}

const TabSection* TabEngine::getCurrentSectionPtr() const
{
    if (currentSectionIndex >= 0 && currentSectionIndex < (int)sections.size())
        return &sections[currentSectionIndex];
    return nullptr;
}

TabPart* TabEngine::getCurrentPartPtr()
{
    auto* section = getCurrentSectionPtr();
    if (!section) return nullptr;

    if (currentPartIndex >= 0 && currentPartIndex < (int)section->parts.size())
        return &section->parts[currentPartIndex];
    return nullptr;
}

const TabPart* TabEngine::getCurrentPartPtr() const
{
    auto* section = getCurrentSectionPtr();
    if (!section) return nullptr;

    if (currentPartIndex >= 0 && currentPartIndex < (int)section->parts.size())
        return &section->parts[currentPartIndex];
    return nullptr;
}

// Part management
int TabEngine::getNumParts() const
{
    auto* section = getCurrentSectionPtr();
    if (section)
        return (int)section->parts.size();
    return 0;
}

void TabEngine::addPart(const juce::String& name)
{
    auto* section = getCurrentSectionPtr();
    if (!section) return;

    section->parts.push_back(TabPart(name, numStrings, 16));
    notifyListeners();
}

void TabEngine::removePart(int partIndex)
{
    auto* section = getCurrentSectionPtr();
    if (!section) return;

    if (partIndex >= 0 && partIndex < (int)section->parts.size() && section->parts.size() > 1)
    {
        section->parts.erase(section->parts.begin() + partIndex);

        // Adjust current part index if needed
        if (currentPartIndex >= (int)section->parts.size())
            currentPartIndex = (int)section->parts.size() - 1;
        if (currentPartIndex < 0)
            currentPartIndex = 0;

        notifyListeners();
    }
}

void TabEngine::renamePart(int partIndex, const juce::String& newName)
{
    auto* section = getCurrentSectionPtr();
    if (!section) return;

    if (partIndex >= 0 && partIndex < (int)section->parts.size())
    {
        section->parts[partIndex].name = newName;
        notifyListeners();
    }
}

juce::String TabEngine::getPartName(int partIndex) const
{
    auto* section = getCurrentSectionPtr();
    if (!section) return "";

    if (partIndex >= 0 && partIndex < (int)section->parts.size())
        return section->parts[partIndex].name;
    return "";
}

void TabEngine::setCurrentPart(int partIndex)
{
    auto* section = getCurrentSectionPtr();
    if (!section) return;

    if (partIndex >= 0 && partIndex < (int)section->parts.size())
    {
        currentPartIndex = partIndex;
        notifyListeners();
    }
}

void TabEngine::clearPart(int partIndex)
{
    auto* section = getCurrentSectionPtr();
    if (!section) return;

    if (partIndex >= 0 && partIndex < (int)section->parts.size())
    {
        auto& part = section->parts[partIndex];
        part.columns.clear();
        part.columns.reserve(16);
        for (int i = 0; i < 16; ++i)
            part.columns.push_back(TabColumn(numStrings));
        notifyListeners();
    }
}

std::unique_ptr<juce::XmlElement> TabEngine::copyPartToXML(int partIndex) const
{
    auto* section = getCurrentSectionPtr();
    if (!section) return nullptr;

    if (partIndex >= 0 && partIndex < (int)section->parts.size())
    {
        auto xml = std::make_unique<juce::XmlElement>("Part");
        const auto& part = section->parts[partIndex];

        xml->setAttribute("name", part.name);
        xml->setAttribute("numColumns", (int)part.columns.size());

        for (int col = 0; col < (int)part.columns.size(); ++col)
        {
            auto* colXml = xml->createNewChildElement("Column");
            colXml->setAttribute("index", col);

            for (int str = 0; str < (int)part.columns[col].notes.size(); ++str)
            {
                const auto& note = part.columns[col].notes[str];
                if (note.fret >= 0)
                {
                    auto* noteXml = colXml->createNewChildElement("Note");
                    noteXml->setAttribute("string", str);
                    noteXml->setAttribute("fret", note.fret);
                    noteXml->setAttribute("technique", (int)note.technique);
                }
            }
        }

        return xml;
    }

    return nullptr;
}

void TabEngine::pastePartFromXML(int partIndex, const juce::XmlElement& xml)
{
    auto* section = getCurrentSectionPtr();
    if (!section) return;

    if (partIndex >= 0 && partIndex < (int)section->parts.size())
    {
        auto& part = section->parts[partIndex];

        // Load part name and columns
        part.name = xml.getStringAttribute("name", "Pasted Part");
        int numCols = xml.getIntAttribute("numColumns", 16);

        // Clear and resize columns
        part.columns.clear();
        part.columns.reserve(numCols);
        for (int i = 0; i < numCols; ++i)
            part.columns.push_back(TabColumn(numStrings));

        // Load notes
        for (auto* colXml : xml.getChildWithTagNameIterator("Column"))
        {
            int colIndex = colXml->getIntAttribute("index", -1);
            if (colIndex >= 0 && colIndex < (int)part.columns.size())
            {
                for (auto* noteXml : colXml->getChildWithTagNameIterator("Note"))
                {
                    int str = noteXml->getIntAttribute("string", -1);
                    int fret = noteXml->getIntAttribute("fret", -1);
                    int tech = noteXml->getIntAttribute("technique", 0);

                    if (str >= 0 && str < (int)part.columns[colIndex].notes.size())
                    {
                        part.columns[colIndex].notes[str].fret = fret;
                        part.columns[colIndex].notes[str].technique = (Technique)tech;
                    }
                }
            }
        }

        notifyListeners();
    }
}

void TabEngine::setNumColumns(int num)
{
    auto* part = getCurrentPartPtr();
    if (!part) return;

    part->columns.clear();
    part->columns.reserve(num);
    for (int i = 0; i < num; ++i)
        part->columns.push_back(TabColumn(numStrings));

    notifyListeners();
}

int TabEngine::getNumColumns() const
{
    auto* part = getCurrentPartPtr();
    if (part)
        return (int)part->columns.size();
    return 0;
}

void TabEngine::setFret(int columnIndex, int stringIndex, int fret)
{
    auto* part = getCurrentPartPtr();
    if (!part) return;

    if (columnIndex >= 0 && columnIndex < (int)part->columns.size())
    {
        part->columns[columnIndex].setFret(stringIndex, fret);
        notifyListeners();
    }
}

int TabEngine::getFret(int columnIndex, int stringIndex) const
{
    auto* part = getCurrentPartPtr();
    if (!part) return -1;

    if (columnIndex >= 0 && columnIndex < (int)part->columns.size())
        return part->columns[columnIndex].getFret(stringIndex);
    return -1;
}

void TabEngine::setTechnique(int columnIndex, int stringIndex, Technique tech, bool beforeFret)
{
    auto* part = getCurrentPartPtr();
    if (!part) return;

    if (columnIndex >= 0 && columnIndex < (int)part->columns.size())
    {
        part->columns[columnIndex].setTechnique(stringIndex, tech, beforeFret);
        notifyListeners();
    }
}

Technique TabEngine::getTechnique(int columnIndex, int stringIndex) const
{
    auto* part = getCurrentPartPtr();
    if (!part) return Technique::None;

    if (columnIndex >= 0 && columnIndex < (int)part->columns.size())
        return part->columns[columnIndex].getTechnique(stringIndex);
    return Technique::None;
}

bool TabEngine::isTechniqueBeforeFret(int columnIndex, int stringIndex) const
{
    auto* part = getCurrentPartPtr();
    if (!part) return false;

    if (columnIndex >= 0 && columnIndex < (int)part->columns.size())
        return part->columns[columnIndex].isTechniqueBeforeFret(stringIndex);
    return false;
}

void TabEngine::insertColumn(int beforeIndex)
{
    auto* part = getCurrentPartPtr();
    if (!part) return;

    if (beforeIndex >= 0 && beforeIndex <= (int)part->columns.size())
    {
        part->columns.insert(part->columns.begin() + beforeIndex, TabColumn(numStrings));
        notifyListeners();
    }
}

void TabEngine::insertBarLine(int beforeIndex)
{
    auto* part = getCurrentPartPtr();
    if (!part) return;

    if (beforeIndex >= 0 && beforeIndex <= (int)part->columns.size())
    {
        part->columns.insert(part->columns.begin() + beforeIndex, TabColumn(numStrings, true));
        notifyListeners();
    }
}

void TabEngine::deleteColumn(int index)
{
    auto* part = getCurrentPartPtr();
    if (!part) return;

    if (index >= 0 && index < (int)part->columns.size() && part->columns.size() > 1)
    {
        part->columns.erase(part->columns.begin() + index);
        notifyListeners();
    }
}

void TabEngine::clearColumn(int index)
{
    auto* part = getCurrentPartPtr();
    if (!part) return;

    if (index >= 0 && index < (int)part->columns.size())
    {
        part->columns[index] = TabColumn(numStrings);
        notifyListeners();
    }
}

bool TabEngine::isColumnBarLine(int index) const
{
    auto* part = getCurrentPartPtr();
    if (!part) return false;

    if (index >= 0 && index < (int)part->columns.size())
        return part->columns[index].isBarLine;
    return false;
}

std::unique_ptr<juce::XmlElement> TabEngine::saveToXML() const
{
    auto xml = std::make_unique<juce::XmlElement>("TabData");

    xml->setAttribute("numStrings", numStrings);
    xml->setAttribute("rootNote", rootNote);
    xml->setAttribute("tuningType", (int)tuningType);
    xml->setAttribute("currentSection", currentSectionIndex);
    xml->setAttribute("currentPart", currentPartIndex);

    // Save custom tuning notes
    juce::String customNotes;
    for (int i = 0; i < (int)customTuning.notes.size(); ++i)
    {
        if (i > 0) customNotes += ",";
        customNotes += customTuning.notes[i];
    }
    xml->setAttribute("customTuning", customNotes);

    // Save all sections
    for (int s = 0; s < (int)sections.size(); ++s)
    {
        auto* sectionXml = xml->createNewChildElement("Section");
        sectionXml->setAttribute("index", s);
        sectionXml->setAttribute("name", sections[s].name);

        // Save all parts in this section
        for (int p = 0; p < (int)sections[s].parts.size(); ++p)
        {
            auto* partXml = sectionXml->createNewChildElement("Part");
            partXml->setAttribute("index", p);
            partXml->setAttribute("name", sections[s].parts[p].name);
            partXml->setAttribute("numColumns", (int)sections[s].parts[p].columns.size());

            for (int col = 0; col < (int)sections[s].parts[p].columns.size(); ++col)
            {
                auto* columnXml = partXml->createNewChildElement("Column");
                columnXml->setAttribute("index", col);
                columnXml->setAttribute("isBarLine", sections[s].parts[p].columns[col].isBarLine);

                for (int str = 0; str < numStrings; ++str)
                {
                    int fret = sections[s].parts[p].columns[col].getFret(str);
                    if (fret >= 0)
                    {
                        auto* noteXml = columnXml->createNewChildElement("Note");
                        noteXml->setAttribute("string", str);
                        noteXml->setAttribute("fret", fret);
                        noteXml->setAttribute("technique", (int)sections[s].parts[p].columns[col].getTechnique(str));
                    }
                }
            }
        }
    }

    return xml;
}

void TabEngine::loadFromXML(const juce::XmlElement& xml)
{
    if (xml.hasTagName("TabData"))
    {
        numStrings = xml.getIntAttribute("numStrings", 6);
        rootNote = xml.getStringAttribute("rootNote", "E");
        tuningType = (TuningType)xml.getIntAttribute("tuningType", (int)TuningType::Standard);
        currentSectionIndex = xml.getIntAttribute("currentSection", 0);
        currentPartIndex = xml.getIntAttribute("currentPart", 0);

        // Load custom tuning
        juce::String customNotes = xml.getStringAttribute("customTuning");
        if (customNotes.isNotEmpty())
        {
            customTuning.notes.clear();
            juce::StringArray noteArray;
            noteArray.addTokens(customNotes, ",", "");
            for (auto& note : noteArray)
                customTuning.notes.push_back(note);
        }
        else
        {
            // Fallback to updating tuning
            updateTuning();
        }

        // Load sections
        sections.clear();
        for (auto* sectionXml : xml.getChildIterator())
        {
            if (sectionXml->hasTagName("Section"))
            {
                juce::String sectionName = sectionXml->getStringAttribute("name", "Untitled");

                // Create section with no columns (we'll add parts)
                TabSection section;
                section.name = sectionName;
                section.parts.clear();

                // Load parts
                for (auto* partXml : sectionXml->getChildIterator())
                {
                    if (partXml->hasTagName("Part"))
                    {
                        juce::String partName = partXml->getStringAttribute("name", "Part 1");
                        int numCols = partXml->getIntAttribute("numColumns", 16);

                        section.parts.push_back(TabPart(partName, numStrings, numCols));
                        int pIndex = (int)section.parts.size() - 1;

                        for (auto* columnXml : partXml->getChildIterator())
                        {
                            if (columnXml->hasTagName("Column"))
                            {
                                int colIndex = columnXml->getIntAttribute("index");
                                bool isBarLine = columnXml->getBoolAttribute("isBarLine", false);

                                if (colIndex >= 0 && colIndex < numCols)
                                {
                                    section.parts[pIndex].columns[colIndex].isBarLine = isBarLine;

                                    for (auto* noteXml : columnXml->getChildIterator())
                                    {
                                        if (noteXml->hasTagName("Note"))
                                        {
                                            int str = noteXml->getIntAttribute("string");
                                            int fret = noteXml->getIntAttribute("fret");
                                            int tech = noteXml->getIntAttribute("technique", (int)Technique::None);
                                            section.parts[pIndex].columns[colIndex].setFret(str, fret);
                                            section.parts[pIndex].columns[colIndex].setTechnique(str, (Technique)tech);
                                        }
                                    }
                                }
                            }
                        }
                    }
                }

                // Ensure section has at least one part
                if (section.parts.empty())
                    section.parts.push_back(TabPart("Part 1", numStrings, 16));

                sections.push_back(section);
            }
        }

        // Ensure we have at least one section
        if (sections.empty())
            sections.push_back(TabSection("Intro", numStrings, 16));

        // Validate current indices
        if (currentSectionIndex >= (int)sections.size())
            currentSectionIndex = 0;

        if (currentSectionIndex >= 0 && currentSectionIndex < (int)sections.size())
        {
            if (currentPartIndex >= (int)sections[currentSectionIndex].parts.size())
                currentPartIndex = 0;
        }

        notifyListeners();
    }
}

juce::String TabEngine::exportToText() const
{
    juce::String output;
    auto tuning = getCurrentTuning();

    // Header - create tuning description
    juce::String tuningDesc = rootNote + " ";
    switch (tuningType)
    {
        case TuningType::Standard: tuningDesc += "Standard"; break;
        case TuningType::Drop: tuningDesc += "Drop"; break;
        case TuningType::Open: tuningDesc += "Open"; break;
        case TuningType::Custom: tuningDesc += "Custom"; break;
    }

    output += "Tuning: " + tuningDesc + " (" + juce::String(numStrings) + " strings)\n\n";

    // Export all sections
    for (int s = 0; s < (int)sections.size(); ++s)
    {
        output += "[ " + sections[s].name + " ]\n\n";

        // Export all parts in this section
        for (int p = 0; p < (int)sections[s].parts.size(); ++p)
        {
            // Show part name if there's more than one part
            if (sections[s].parts.size() > 1)
                output += "  " + sections[s].parts[p].name + "\n\n";

            // Draw each string
            for (int str = numStrings - 1; str >= 0; --str)
            {
                // String name
                if (str < (int)tuning.notes.size())
                    output += tuning.notes[str].paddedRight(' ', 3) + "|-";
                else
                    output += "----";

                // Notes on this string
                for (int col = 0; col < (int)sections[s].parts[p].columns.size(); ++col)
                {
                    bool isBarLine = sections[s].parts[p].columns[col].isBarLine;

                    // If this column is marked as a bar line, insert the bar line visual first
                    if (isBarLine)
                    {
                        output += "-|-";
                    }

                    // Then always render the note data (bar line columns can also hold notes)
                    int fret = sections[s].parts[p].columns[col].getFret(str);
                    Technique tech = sections[s].parts[p].columns[col].getTechnique(str);
                    bool beforeFret = sections[s].parts[p].columns[col].isTechniqueBeforeFret(str);

                    if (fret >= 0)
                    {
                        juce::String fretStr = juce::String(fret);
                        juce::String techStr = "";

                        // Get technique symbol
                        if (tech == Technique::HammerOn)
                            techStr = "h";
                        else if (tech == Technique::PullOff)
                            techStr = "p";
                        else if (tech == Technique::SlideUp)
                            techStr = "/";
                        else if (tech == Technique::SlideDown)
                            techStr = "\\";
                        else if (tech == Technique::Bend)
                            techStr = "b";
                        else if (tech == Technique::ReleaseBend)
                            techStr = "r";
                        else if (tech == Technique::Tap)
                            techStr = "t";
                        else if (tech == Technique::Vibrato)
                            techStr = "~";

                        // Special cases that don't follow before/after pattern
                        if (tech == Technique::Mute)
                        {
                            fretStr = "x";
                        }
                        else if (tech == Technique::Harmonic)
                        {
                            fretStr = "<" + fretStr + ">";
                        }
                        else if (!techStr.isEmpty())
                        {
                            // Apply technique before or after based on flag
                            if (beforeFret)
                                fretStr = techStr + fretStr;
                            else
                                fretStr = fretStr + techStr;
                        }

                        // Determine padding width based on the actual string length
                        // Maximum expected: <24> = 4 chars, or 24h = 3 chars
                        // Use 4 characters as standard width for all cells
                        int padWidth = 4;
                        output += fretStr.paddedLeft('-', padWidth);
                    }
                    else
                    {
                        // Empty cell - use same padding as fret cells
                        output += "----";
                    }
                }

                // Add spacing before end bar
                output += "-";

                output += "|\n";
            }

            output += "\n";
        }
    }

    return output;
}

void TabEngine::addListener(Listener* listener)
{
    listeners.add(listener);
}

void TabEngine::removeListener(Listener* listener)
{
    listeners.remove(listener);
}

void TabEngine::notifyListeners()
{
    listeners.call(&Listener::tabDataChanged);
}
