#pragma once

#include <JuceHeader.h>
#include "PluginProcessor.h"
#include "TabEditorComponent.h"
#include "KeyboardShortcutsPanel.h"

//==============================================================================
class TabVSTAudioProcessorEditor : public juce::AudioProcessorEditor
{
public:
    TabVSTAudioProcessorEditor (TabVSTAudioProcessor&);
    ~TabVSTAudioProcessorEditor() override;

    //==============================================================================
    void paint (juce::Graphics&) override;
    void resized() override;
    void mouseDown(const juce::MouseEvent& event) override;

private:
    TabVSTAudioProcessor& audioProcessor;

    // UI Components
    TabEditorComponent tabEditor;
    juce::Viewport tabEditorViewport;

    // Tuning controls
    juce::Label stringsLabel;
    juce::ComboBox stringsSelector;

    juce::Label keyLabel;
    juce::ComboBox keySelector;

    juce::Label tuningTypeLabel;
    juce::ComboBox tuningTypeSelector;

    // Custom tuning controls (shown when Custom is selected)
    std::vector<std::unique_ptr<juce::ComboBox>> customStringSelectors;
    std::vector<std::unique_ptr<juce::Label>> customStringLabels;

    juce::TextButton exportButton;
    juce::TextButton exportFileButton;
    juce::TextButton importFileButton;
    juce::TextButton addColumnsButton;
    juce::TextButton removeColumnsButton;
    juce::TextButton modeButton;
    juce::TextEditor asciiView;

    // Keyboard shortcuts panel
    juce::TextButton shortcutsButton;
    std::unique_ptr<KeyboardShortcutsPanel> shortcutsPanel;
    bool isShortcutsPanelVisible = false;

    // Separator labels
    juce::Label instrumentSetupLabel;
    juce::Label arrangerLabel;

    // Section management controls
    std::vector<std::unique_ptr<juce::TextButton>> sectionButtons;
    juce::TextButton addSectionButton;
    juce::TextButton removeSectionButton;

    // Part management controls
    std::vector<std::unique_ptr<juce::TextButton>> partButtons;
    juce::TextButton addPartButton;
    juce::TextButton removePartButton;

    void setupControls();
    void syncUIWithEngine();
    void stringsChanged();
    void keyChanged();
    void tuningTypeChanged();
    void customStringNoteChanged(int stringIndex);
    void updateCustomTuningControls();
    void exportToClipboard();
    void exportToFile();
    void importFromFile();
    void addColumn();
    void removeColumn();
    void addBarLine();
    void toggleMode();
    void updateAsciiView();
    void updateSectionButtons();
    void sectionButtonClicked(int sectionIndex);
    void sectionButtonDoubleClicked(int sectionIndex);
    void sectionButtonRightClicked(int sectionIndex);
    void addSection();
    void removeSection();
    void renameSection(int sectionIndex);
    void clearSection(int sectionIndex);
    void copySection(int sectionIndex);
    void pasteSection(int sectionIndex);
    void updatePartButtons();
    void partButtonClicked(int partIndex);
    void partButtonDoubleClicked(int partIndex);
    void partButtonRightClicked(int partIndex);
    void addPart();
    void removePart();
    void renamePart(int partIndex);
    void clearPart(int partIndex);
    void copyPart(int partIndex);
    void pastePart(int partIndex);
    void toggleShortcutsPanel();

    bool isEditorMode = true;

    // Clipboard for section and part data
    std::unique_ptr<juce::XmlElement> sectionClipboard;
    std::unique_ptr<juce::XmlElement> partClipboard;

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (TabVSTAudioProcessorEditor)
};
