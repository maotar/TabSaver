#pragma once

#include <JuceHeader.h>
#include <vector>
#include <map>

//==============================================================================
// Note utilities
class NoteUtils
{
public:
    static juce::StringArray getAllNotes()
    {
        return { "C", "C#", "D", "D#", "E", "F", "F#", "G", "G#", "A", "A#", "B" };
    }

    static int getNoteIndex(const juce::String& note)
    {
        auto notes = getAllNotes();
        return notes.indexOf(note);
    }

    // Get note that is 'semitones' higher than base note
    static juce::String transposeNote(const juce::String& note, int semitones)
    {
        auto notes = getAllNotes();
        int index = notes.indexOf(note);
        if (index < 0) return note;

        index = (index + semitones) % 12;
        if (index < 0) index += 12;

        return notes[index];
    }
};

//==============================================================================
enum class TuningType
{
    Standard,  // Root, +5, +10, +15, +19, +24 semitones
    Drop,      // Root-2, +5, +10, +15, +19, +24
    Open,      // Custom open tuning
    Custom     // Fully custom
};

//==============================================================================
struct GuitarTuning
{
    std::vector<juce::String> notes; // From lowest to highest string

    GuitarTuning() {}
    GuitarTuning(const std::vector<juce::String>& n) : notes(n) {}

    // Generate standard tuning for given root note and string count
    static GuitarTuning createStandard(const juce::String& rootNote, int numStrings)
    {
        GuitarTuning tuning;
        // Standard intervals: 0, 5, 10, 15, 19, 24, 29, 34, 39 semitones (up to 9 strings)
        juce::Array<int> intervals = { 0, 5, 10, 15, 19, 24, 29, 34, 39 };

        for (int i = 0; i < numStrings && i < intervals.size(); ++i)
            tuning.notes.push_back(NoteUtils::transposeNote(rootNote, intervals[i]));

        return tuning;
    }

    // Generate drop tuning - rootNote is the dropped string note (e.g., "D" for Drop D)
    // The rest follows standard tuning from 2 semitones up
    static GuitarTuning createDrop(const juce::String& rootNote, int numStrings)
    {
        GuitarTuning tuning;
        // First string is the root (dropped string)
        tuning.notes.push_back(rootNote);

        // Rest follows standard intervals from 2 semitones above the drop note
        juce::String standardRoot = NoteUtils::transposeNote(rootNote, 2);
        juce::Array<int> intervals = { 5, 10, 15, 19, 24, 29, 34, 39 }; // Skip first (0)

        for (int i = 1; i < numStrings && (i-1) < intervals.size(); ++i)
            tuning.notes.push_back(NoteUtils::transposeNote(standardRoot, intervals[i-1]));

        return tuning;
    }

    // Generate open tuning (major chord)
    static GuitarTuning createOpen(const juce::String& rootNote, int numStrings)
    {
        GuitarTuning tuning;
        // Open major chord intervals: Root, 5th, Root, 3rd, 5th, Root, 5th, Root, 3rd
        juce::Array<int> intervals = { 0, 7, 12, 16, 19, 24, 31, 36, 40 };

        for (int i = 0; i < numStrings && i < intervals.size(); ++i)
            tuning.notes.push_back(NoteUtils::transposeNote(rootNote, intervals[i]));

        return tuning;
    }
};

//==============================================================================
enum class Technique
{
    None,
    HammerOn,   // h
    PullOff,    // p
    SlideUp,    // /
    SlideDown,  // backslash
    Bend,       // b
    ReleaseBend,// r
    Tap,        // t
    Mute,       // x
    Vibrato,    // ~
    Harmonic    // < or >
};

//==============================================================================
struct TabNote
{
    int stringIndex;  // Which string (0 = lowest)
    int fret;         // Fret number (-1 for empty)
    Technique technique; // Technique applied to this note
    bool techniqueBeforeFret; // True if technique appears before fret (e.g., /5 vs 5/)

    TabNote() : stringIndex(0), fret(-1), technique(Technique::None), techniqueBeforeFret(false) {}
    TabNote(int string, int fretNum) : stringIndex(string), fret(fretNum), technique(Technique::None), techniqueBeforeFret(false) {}

    bool isEmpty() const { return fret < 0; }
};

//==============================================================================
struct TabColumn
{
    std::vector<TabNote> notes; // One note per string
    bool isBarLine; // True if this column is a bar line separator

    TabColumn(int numStrings = 6, bool barLine = false)
        : isBarLine(barLine)
    {
        notes.resize(numStrings);
        for (int i = 0; i < numStrings; ++i)
            notes[i] = TabNote(i, -1);
    }

    void setFret(int stringIndex, int fret)
    {
        if (stringIndex >= 0 && stringIndex < (int)notes.size())
            notes[stringIndex].fret = fret;
    }

    int getFret(int stringIndex) const
    {
        if (stringIndex >= 0 && stringIndex < (int)notes.size())
            return notes[stringIndex].fret;
        return -1;
    }

    void setTechnique(int stringIndex, Technique tech, bool beforeFret = false)
    {
        if (stringIndex >= 0 && stringIndex < (int)notes.size())
        {
            notes[stringIndex].technique = tech;
            notes[stringIndex].techniqueBeforeFret = beforeFret;
        }
    }

    Technique getTechnique(int stringIndex) const
    {
        if (stringIndex >= 0 && stringIndex < (int)notes.size())
            return notes[stringIndex].technique;
        return Technique::None;
    }

    bool isTechniqueBeforeFret(int stringIndex) const
    {
        if (stringIndex >= 0 && stringIndex < (int)notes.size())
            return notes[stringIndex].techniqueBeforeFret;
        return false;
    }
};

//==============================================================================
struct TabPart
{
    juce::String name;
    std::vector<TabColumn> columns;

    TabPart(const juce::String& partName = "Part 1", int numStrings = 6, int numCols = 16)
        : name(partName)
    {
        columns.reserve(numCols);
        for (int i = 0; i < numCols; ++i)
            columns.push_back(TabColumn(numStrings));
    }
};

//==============================================================================
struct TabSection
{
    juce::String name;
    std::vector<TabPart> parts;

    TabSection(const juce::String& sectionName = "Untitled", int numStrings = 6, int numCols = 16)
        : name(sectionName)
    {
        // Start with one default part
        parts.push_back(TabPart("Part 1", numStrings, numCols));
    }
};

//==============================================================================
class TabEngine
{
public:
    TabEngine();
    ~TabEngine();

    // Tab structure
    void setNumStrings(int num);
    int getNumStrings() const { return numStrings; }

    // New tuning system
    void setRootNote(const juce::String& note);
    juce::String getRootNote() const { return rootNote; }

    void setTuningType(TuningType type);
    TuningType getTuningType() const { return tuningType; }

    void setCustomStringNote(int stringIndex, const juce::String& note);
    juce::String getStringNote(int stringIndex) const;

    GuitarTuning getCurrentTuning() const;

    // Section management
    int getNumSections() const { return (int)sections.size(); }
    void addSection(const juce::String& name = "New Section");
    void removeSection(int sectionIndex);
    void renameSection(int sectionIndex, const juce::String& newName);
    juce::String getSectionName(int sectionIndex) const;
    void setCurrentSection(int sectionIndex);
    int getCurrentSection() const { return currentSectionIndex; }
    void clearSection(int sectionIndex);
    std::unique_ptr<juce::XmlElement> copySectionToXML(int sectionIndex) const;
    void pasteSectionFromXML(int sectionIndex, const juce::XmlElement& xml);

    // Part management (within current section)
    int getNumParts() const;
    void addPart(const juce::String& name = "New Part");
    void removePart(int partIndex);
    void renamePart(int partIndex, const juce::String& newName);
    juce::String getPartName(int partIndex) const;
    void setCurrentPart(int partIndex);
    int getCurrentPart() const { return currentPartIndex; }
    void clearPart(int partIndex);
    std::unique_ptr<juce::XmlElement> copyPartToXML(int partIndex) const;
    void pastePartFromXML(int partIndex, const juce::XmlElement& xml);

    void setNumColumns(int num);
    int getNumColumns() const;

    // Tab editing (operates on current section)
    void setFret(int columnIndex, int stringIndex, int fret);
    int getFret(int columnIndex, int stringIndex) const;

    void setTechnique(int columnIndex, int stringIndex, Technique tech, bool beforeFret = false);
    Technique getTechnique(int columnIndex, int stringIndex) const;
    bool isTechniqueBeforeFret(int columnIndex, int stringIndex) const;

    void insertColumn(int beforeIndex);
    void insertBarLine(int beforeIndex);
    void deleteColumn(int index);
    void clearColumn(int index);
    bool isColumnBarLine(int index) const;

    // Save/Load
    std::unique_ptr<juce::XmlElement> saveToXML() const;
    void loadFromXML(const juce::XmlElement& xml);

    // Export to text
    juce::String exportToText() const;

    // Listeners
    class Listener
    {
    public:
        virtual ~Listener() = default;
        virtual void tabDataChanged() = 0;
    };

    void addListener(Listener* listener);
    void removeListener(Listener* listener);

private:
    int numStrings;
    juce::String rootNote;
    TuningType tuningType;
    GuitarTuning customTuning;
    std::vector<TabSection> sections;
    int currentSectionIndex;
    int currentPartIndex;
    juce::ListenerList<Listener> listeners;

    void notifyListeners();
    void updateTuning();
    TabSection* getCurrentSectionPtr();
    const TabSection* getCurrentSectionPtr() const;
    TabPart* getCurrentPartPtr();
    const TabPart* getCurrentPartPtr() const;

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (TabEngine)
};
