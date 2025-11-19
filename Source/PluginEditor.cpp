#include "PluginProcessor.h"
#include "PluginEditor.h"

//==============================================================================
TabVSTAudioProcessorEditor::TabVSTAudioProcessorEditor (TabVSTAudioProcessor& p)
    : AudioProcessorEditor (&p), audioProcessor (p), tabEditor(p.getTabEngine())
{
    setSize (1000, 600);
    setResizable(true, true);
    setResizeLimits(800, 400, 4000, 2000);

    setupControls();
    syncUIWithEngine();
}

TabVSTAudioProcessorEditor::~TabVSTAudioProcessorEditor()
{
}

void TabVSTAudioProcessorEditor::setupControls()
{
    // Strings selector (first)
    addAndMakeVisible(stringsLabel);
    stringsLabel.setText("Strings:", juce::dontSendNotification);
    stringsLabel.setJustificationType(juce::Justification::centredRight);

    addAndMakeVisible(stringsSelector);
    for (int i = 4; i <= 9; ++i)
        stringsSelector.addItem(juce::String(i), i);
    stringsSelector.setSelectedId(6, juce::dontSendNotification);
    stringsSelector.onChange = [this] { stringsChanged(); };

    // Key selector (E at top, then descending musically)
    addAndMakeVisible(keyLabel);
    keyLabel.setText("Key:", juce::dontSendNotification);
    keyLabel.setJustificationType(juce::Justification::centredRight);

    addAndMakeVisible(keySelector);
    // E, then descending: D#, D, C#, C, B, A#, A, G#, G, F#, F
    juce::StringArray orderedNotes = { "E", "D#", "D", "C#", "C", "B", "A#", "A", "G#", "G", "F#", "F" };
    for (int i = 0; i < orderedNotes.size(); ++i)
        keySelector.addItem(orderedNotes[i], i + 1);
    keySelector.setSelectedId(1, juce::dontSendNotification); // E is ID 1
    keySelector.onChange = [this] { keyChanged(); };

    // Tuning type selector
    addAndMakeVisible(tuningTypeLabel);
    tuningTypeLabel.setText("Type:", juce::dontSendNotification);
    tuningTypeLabel.setJustificationType(juce::Justification::centredRight);

    addAndMakeVisible(tuningTypeSelector);
    tuningTypeSelector.addItem("Standard", 1);
    tuningTypeSelector.addItem("Drop", 2);
    tuningTypeSelector.addItem("Open", 3);
    tuningTypeSelector.addItem("Custom", 4);
    tuningTypeSelector.setSelectedId(1, juce::dontSendNotification);
    tuningTypeSelector.onChange = [this] { tuningTypeChanged(); };

    // Export button
    addAndMakeVisible(exportButton);
    exportButton.setButtonText("Copy Tab");
    exportButton.setTooltip("Copy to Clipboard");
    exportButton.onClick = [this] { exportToClipboard(); };
    exportButton.setVisible(false); // Initially hidden (starts in editor mode)

    // Export to file button
    addAndMakeVisible(exportFileButton);
    exportFileButton.setButtonText("Export");
    exportFileButton.setTooltip("Export to File");
    exportFileButton.onClick = [this] { exportToFile(); };
    exportFileButton.setVisible(false); // Initially hidden (starts in editor mode)

    // Import from file button
    addAndMakeVisible(importFileButton);
    importFileButton.setButtonText("Import");
    importFileButton.setTooltip("Import from File");
    importFileButton.onClick = [this] { importFromFile(); };
    importFileButton.setVisible(false); // Initially hidden (starts in editor mode)

    // Clear button - removed, now available via right-click on sections

    // Add/Remove columns buttons removed - functionality moved to part right-click menu

    // Mode toggle button
    addAndMakeVisible(modeButton);
    modeButton.setButtonText("View Tab");
    modeButton.setTooltip("Toggle Editor/ASCII View");
    modeButton.onClick = [this] { toggleMode(); };

    // ASCII view (hidden initially)
    addAndMakeVisible(asciiView);
    asciiView.setMultiLine(true);
    asciiView.setReadOnly(true);
    asciiView.setScrollbarsShown(true);
    asciiView.setFont(juce::FontOptions(juce::Font::getDefaultMonospacedFontName(), 14.0f, juce::Font::plain));
    asciiView.setColour(juce::TextEditor::backgroundColourId, juce::Colour(0xff1a1a1a));
    asciiView.setColour(juce::TextEditor::textColourId, juce::Colours::lightgrey);
    asciiView.setVisible(false);

    // Keyboard shortcuts button
    addAndMakeVisible(shortcutsButton);
    shortcutsButton.setButtonText("?");
    shortcutsButton.setTooltip("Show keyboard shortcuts");
    shortcutsButton.onClick = [this] { toggleShortcutsPanel(); };

    // Tab editor with viewport for scrolling
    addAndMakeVisible(tabEditorViewport);
    tabEditorViewport.setViewedComponent(&tabEditor, false);
    tabEditorViewport.setScrollBarsShown(false, true); // Vertical: auto, Horizontal: always

    // Separator labels
    addAndMakeVisible(instrumentSetupLabel);
    instrumentSetupLabel.setText("Instrument Setup", juce::dontSendNotification);
    instrumentSetupLabel.setJustificationType(juce::Justification::centredLeft);
    instrumentSetupLabel.setFont(juce::FontOptions(13.0f, juce::Font::bold));
    instrumentSetupLabel.setColour(juce::Label::textColourId, juce::Colours::lightblue);

    addAndMakeVisible(arrangerLabel);
    arrangerLabel.setText("Arranger", juce::dontSendNotification);
    arrangerLabel.setJustificationType(juce::Justification::centredLeft);
    arrangerLabel.setFont(juce::FontOptions(13.0f, juce::Font::bold));
    arrangerLabel.setColour(juce::Label::textColourId, juce::Colours::lightblue);

    // Section management buttons
    addAndMakeVisible(addSectionButton);
    addSectionButton.setButtonText("+");
    addSectionButton.setTooltip("Add Section");
    addSectionButton.onClick = [this] { addSection(); };

    addAndMakeVisible(removeSectionButton);
    removeSectionButton.setButtonText("-");
    removeSectionButton.setTooltip("Remove Section");
    removeSectionButton.onClick = [this] { removeSection(); };

    // Part management buttons
    addAndMakeVisible(addPartButton);
    addPartButton.setButtonText("+");
    addPartButton.setTooltip("Add Part");
    addPartButton.onClick = [this] { addPart(); };

    addAndMakeVisible(removePartButton);
    removePartButton.setButtonText("-");
    removePartButton.setTooltip("Remove Part");
    removePartButton.onClick = [this] { removePart(); };

    // Initialize custom tuning controls (hidden initially)
    updateCustomTuningControls();

    // Initialize section and part buttons
    updateSectionButtons();
    updatePartButtons();
}

void TabVSTAudioProcessorEditor::syncUIWithEngine()
{
    // Sync strings selector
    int numStrings = audioProcessor.getTabEngine().getNumStrings();
    stringsSelector.setSelectedId(numStrings, juce::dontSendNotification);

    // Sync key selector
    juce::String rootNote = audioProcessor.getTabEngine().getRootNote();
    juce::StringArray orderedNotes = { "E", "D#", "D", "C#", "C", "B", "A#", "A", "G#", "G", "F#", "F" };
    int noteIndex = orderedNotes.indexOf(rootNote);
    if (noteIndex >= 0)
        keySelector.setSelectedId(noteIndex + 1, juce::dontSendNotification);

    // Sync tuning type selector
    TuningType tuningType = audioProcessor.getTabEngine().getTuningType();
    int typeId = 1;
    switch (tuningType)
    {
        case TuningType::Standard: typeId = 1; break;
        case TuningType::Drop: typeId = 2; break;
        case TuningType::Open: typeId = 3; break;
        case TuningType::Custom: typeId = 4; break;
    }
    tuningTypeSelector.setSelectedId(typeId, juce::dontSendNotification);

    // Update custom tuning controls and layout
    updateCustomTuningControls();
    updateSectionButtons();
    updatePartButtons();
    resized();
}

void TabVSTAudioProcessorEditor::paint (juce::Graphics& g)
{
    g.fillAll(juce::Colour(0xff2a2a2a));
}

void TabVSTAudioProcessorEditor::mouseDown(const juce::MouseEvent& event)
{
    // Close shortcuts panel if clicking outside of it and the shortcuts button
    if (isShortcutsPanelVisible && shortcutsPanel)
    {
        auto panelBounds = shortcutsPanel->getBounds();
        auto buttonBounds = shortcutsButton.getBounds();
        auto clickPos = event.getPosition();

        if (!panelBounds.contains(clickPos) && !buttonBounds.contains(clickPos))
        {
            isShortcutsPanelVisible = false;
            shortcutsPanel->setVisible(false);
        }
    }
}

void TabVSTAudioProcessorEditor::resized()
{
    auto bounds = getLocalBounds();

    // Top bar with Instrument Setup label on left and mode button on right
    auto topBar = bounds.removeFromTop(25);
    topBar.removeFromTop(5);

    // Right side: mode button (with more padding from edge)
    topBar.removeFromRight(20); // Add padding from edge
    modeButton.setBounds(topBar.removeFromRight(85));

    // Left side: Instrument Setup label
    topBar.removeFromLeft(10);
    instrumentSetupLabel.setBounds(topBar);

    // Instrument Setup controls bar
    auto instrumentBar = bounds.removeFromTop(35);
    instrumentBar.removeFromLeft(10);
    instrumentBar.removeFromTop(5);

    // Left side: Strings, Key, Type
    stringsLabel.setBounds(instrumentBar.removeFromLeft(55));
    stringsSelector.setBounds(instrumentBar.removeFromLeft(60).withTrimmedRight(10));

    keyLabel.setBounds(instrumentBar.removeFromLeft(40));
    keySelector.setBounds(instrumentBar.removeFromLeft(60).withTrimmedRight(10));

    tuningTypeLabel.setBounds(instrumentBar.removeFromLeft(45));
    tuningTypeSelector.setBounds(instrumentBar.removeFromLeft(100).withTrimmedRight(10));

    // Custom tuning controls area (if visible)
    if (tuningTypeSelector.getSelectedId() == 4) // Custom
    {
        auto customBar = bounds.removeFromTop(40);
        customBar.removeFromLeft(10);
        customBar.removeFromTop(5);

        int numStrings = audioProcessor.getTabEngine().getNumStrings();
        int controlWidth = 70;

        for (int i = numStrings - 1; i >= 0; --i)
        {
            if (i < customStringLabels.size() && customStringLabels[i])
                customStringLabels[i]->setBounds(customBar.removeFromLeft(45));
            if (i < customStringSelectors.size() && customStringSelectors[i])
                customStringSelectors[i]->setBounds(customBar.removeFromLeft(controlWidth).withTrimmedRight(5));
        }
    }

    // Add spacing before Arranger section
    bounds.removeFromTop(10);

    // Arranger separator
    auto arrangerBar = bounds.removeFromTop(25);
    arrangerBar.removeFromLeft(10);
    arrangerLabel.setBounds(arrangerBar);

    // Section buttons area
    auto sectionBar = bounds.removeFromTop(40);
    sectionBar.removeFromLeft(10);
    sectionBar.removeFromTop(5);

    // Add/Remove section buttons on the left
    addSectionButton.setBounds(sectionBar.removeFromLeft(35).withTrimmedRight(5));
    removeSectionButton.setBounds(sectionBar.removeFromLeft(35).withTrimmedRight(10));

    // Section tab buttons
    int numSections = audioProcessor.getTabEngine().getNumSections();
    for (int i = 0; i < numSections && i < sectionButtons.size(); ++i)
    {
        if (sectionButtons[i])
        {
            int buttonWidth = 100;
            sectionButtons[i]->setBounds(sectionBar.removeFromLeft(buttonWidth).withTrimmedRight(5));
        }
    }

    // Part buttons area
    auto partBar = bounds.removeFromTop(35);
    partBar.removeFromLeft(10);
    partBar.removeFromTop(5);

    // Add/Remove part buttons on the left
    addPartButton.setBounds(partBar.removeFromLeft(35).withTrimmedRight(5));
    removePartButton.setBounds(partBar.removeFromLeft(35).withTrimmedRight(10));

    // Part tab buttons
    int numParts = audioProcessor.getTabEngine().getNumParts();
    for (int i = 0; i < numParts && i < partButtons.size(); ++i)
    {
        if (partButtons[i])
        {
            int buttonWidth = 100;
            partButtons[i]->setBounds(partBar.removeFromLeft(buttonWidth).withTrimmedRight(5));
        }
    }

    // Add spacing before editor/view component
    bounds.removeFromTop(10);

    // Bottom bar with shortcuts button and action buttons
    auto helpBar = bounds.removeFromBottom(30);
    helpBar.removeFromBottom(10); // Push button up by removing space from bottom

    if (isEditorMode)
    {
        // In editor mode: only shortcuts button on left, hide export buttons
        helpBar.removeFromLeft(10);
        shortcutsButton.setBounds(helpBar.removeFromLeft(30).withTrimmedRight(5));
        exportButton.setBounds(0, 0, 0, 0);
        exportFileButton.setBounds(0, 0, 0, 0);
        importFileButton.setBounds(0, 0, 0, 0);
    }
    else
    {
        // In view mode: shortcuts button on left, action buttons on right
        // Layout: [?] ... [Import] [Export] [Copy Tab]
        helpBar.removeFromRight(20); // Padding from edge
        exportButton.setBounds(helpBar.removeFromRight(85));
        exportFileButton.setBounds(helpBar.removeFromRight(75));
        importFileButton.setBounds(helpBar.removeFromRight(75));

        helpBar.removeFromLeft(10);
        shortcutsButton.setBounds(helpBar.removeFromLeft(30).withTrimmedRight(5));
    }

    // Position shortcuts panel if visible
    if (isShortcutsPanelVisible && shortcutsPanel)
    {
        auto panelBounds = getLocalBounds();
        int panelX = 10;
        int panelY = panelBounds.getHeight() - 30 - shortcutsPanel->getHeight() - 10;
        shortcutsPanel->setBounds(panelX, panelY, shortcutsPanel->getWidth(), shortcutsPanel->getHeight());
    }

    // Tab editor or ASCII view fills remaining space
    if (isEditorMode)
    {
        tabEditorViewport.setBounds(bounds.reduced(10));
        asciiView.setBounds(0, 0, 0, 0); // Hide
    }
    else
    {
        asciiView.setBounds(bounds.reduced(10));
        tabEditorViewport.setBounds(0, 0, 0, 0); // Hide
    }
}

void TabVSTAudioProcessorEditor::stringsChanged()
{
    int numStrings = stringsSelector.getSelectedId();
    audioProcessor.getTabEngine().setNumStrings(numStrings);
    updateCustomTuningControls();
    resized();
}

void TabVSTAudioProcessorEditor::keyChanged()
{
    juce::StringArray orderedNotes = { "E", "D#", "D", "C#", "C", "B", "A#", "A", "G#", "G", "F#", "F" };
    int selectedId = keySelector.getSelectedId();

    if (selectedId > 0 && selectedId <= orderedNotes.size())
    {
        juce::String selectedNote = orderedNotes[selectedId - 1];
        audioProcessor.getTabEngine().setRootNote(selectedNote);
    }
}

void TabVSTAudioProcessorEditor::tuningTypeChanged()
{
    int typeId = tuningTypeSelector.getSelectedId();
    TuningType type = TuningType::Standard;

    switch (typeId)
    {
        case 1: type = TuningType::Standard; break;
        case 2: type = TuningType::Drop; break;
        case 3: type = TuningType::Open; break;
        case 4: type = TuningType::Custom; break;
    }

    audioProcessor.getTabEngine().setTuningType(type);
    updateCustomTuningControls();
    resized();
}

void TabVSTAudioProcessorEditor::customStringNoteChanged(int stringIndex)
{
    if (stringIndex >= 0 && stringIndex < customStringSelectors.size())
    {
        auto& selector = customStringSelectors[stringIndex];
        if (selector)
        {
            int noteIndex = selector->getSelectedId() - 1;
            auto notes = NoteUtils::getAllNotes();
            if (noteIndex >= 0 && noteIndex < notes.size())
                audioProcessor.getTabEngine().setCustomStringNote(stringIndex, notes[noteIndex]);
        }
    }
}

void TabVSTAudioProcessorEditor::updateCustomTuningControls()
{
    bool isCustom = (tuningTypeSelector.getSelectedId() == 4);
    int numStrings = audioProcessor.getTabEngine().getNumStrings();

    // Clear existing controls
    for (auto& selector : customStringSelectors)
    {
        if (selector)
            removeChildComponent(selector.get());
    }
    for (auto& label : customStringLabels)
    {
        if (label)
            removeChildComponent(label.get());
    }

    customStringSelectors.clear();
    customStringLabels.clear();

    if (isCustom)
    {
        auto notes = NoteUtils::getAllNotes();

        // Create controls for each string (display from highest to lowest)
        for (int i = 0; i < numStrings; ++i)
        {
            // Label
            auto label = std::make_unique<juce::Label>();
            label->setText("S" + juce::String(i + 1) + ":", juce::dontSendNotification);
            label->setJustificationType(juce::Justification::centredRight);
            addAndMakeVisible(label.get());
            customStringLabels.push_back(std::move(label));

            // Selector
            auto selector = std::make_unique<juce::ComboBox>();
            for (int n = 0; n < notes.size(); ++n)
                selector->addItem(notes[n], n + 1);

            // Set current note
            juce::String currentNote = audioProcessor.getTabEngine().getStringNote(i);
            int currentIndex = notes.indexOf(currentNote);
            if (currentIndex >= 0)
                selector->setSelectedId(currentIndex + 1, juce::dontSendNotification);

            int stringIndex = i;
            selector->onChange = [this, stringIndex] { customStringNoteChanged(stringIndex); };

            addAndMakeVisible(selector.get());
            customStringSelectors.push_back(std::move(selector));
        }
    }
}

void TabVSTAudioProcessorEditor::exportToClipboard()
{
    juce::String tabText = audioProcessor.getTabEngine().exportToText();
    juce::SystemClipboard::copyTextToClipboard(tabText);

    // Show confirmation
    juce::NativeMessageBox::showMessageBoxAsync(
        juce::MessageBoxIconType::InfoIcon,
        "Exported",
        "Tab copied to clipboard!");
}

void TabVSTAudioProcessorEditor::exportToFile()
{
    auto fileChooser = std::make_shared<juce::FileChooser>(
        "Export Tab",
        juce::File::getSpecialLocation(juce::File::userDocumentsDirectory),
        "*.tabsaver");

    auto flags = juce::FileBrowserComponent::saveMode | juce::FileBrowserComponent::canSelectFiles;

    fileChooser->launchAsync(flags, [this, fileChooser](const juce::FileChooser& chooser)
    {
        auto file = chooser.getResult();
        if (file != juce::File())
        {
            // Get XML from TabEngine
            auto xml = audioProcessor.getTabEngine().saveToXML();

            // Write to file
            if (xml->writeTo(file))
            {
                juce::NativeMessageBox::showMessageBoxAsync(
                    juce::MessageBoxIconType::InfoIcon,
                    "Exported",
                    "Tab exported successfully to:\n" + file.getFullPathName());
            }
            else
            {
                juce::NativeMessageBox::showMessageBoxAsync(
                    juce::MessageBoxIconType::WarningIcon,
                    "Export Failed",
                    "Failed to write file:\n" + file.getFullPathName());
            }
        }
    });
}

void TabVSTAudioProcessorEditor::importFromFile()
{
    auto fileChooser = std::make_shared<juce::FileChooser>(
        "Import Tab",
        juce::File::getSpecialLocation(juce::File::userDocumentsDirectory),
        "*.tabsaver;*.xml");

    auto flags = juce::FileBrowserComponent::openMode | juce::FileBrowserComponent::canSelectFiles;

    fileChooser->launchAsync(flags, [this, fileChooser](const juce::FileChooser& chooser)
    {
        auto file = chooser.getResult();
        if (file != juce::File())
        {
            // Parse XML from file
            auto xml = juce::parseXML(file);

            if (xml != nullptr)
            {
                // Load into TabEngine
                audioProcessor.getTabEngine().loadFromXML(*xml);

                // Update UI
                syncUIWithEngine();
                tabEditor.repaint();

                // Update ASCII view if in view mode
                if (!isEditorMode)
                    updateAsciiView();

                juce::NativeMessageBox::showMessageBoxAsync(
                    juce::MessageBoxIconType::InfoIcon,
                    "Imported",
                    "Tab imported successfully from:\n" + file.getFullPathName());
            }
            else
            {
                juce::NativeMessageBox::showMessageBoxAsync(
                    juce::MessageBoxIconType::WarningIcon,
                    "Import Failed",
                    "Failed to parse file:\n" + file.getFullPathName());
            }
        }
    });
}

void TabVSTAudioProcessorEditor::addColumn()
{
    // Add 1 column after cursor position
    int cursorColumn = tabEditor.getCursorColumn();
    audioProcessor.getTabEngine().insertColumn(cursorColumn + 1);
    tabEditor.setCursorPosition(cursorColumn + 1, 0);
    tabEditor.repaint();
}

void TabVSTAudioProcessorEditor::removeColumn()
{
    // Remove 1 column at cursor position
    int currentColumns = audioProcessor.getTabEngine().getNumColumns();
    int cursorColumn = tabEditor.getCursorColumn();
    if (currentColumns > 1 && cursorColumn >= 0 && cursorColumn < currentColumns)
    {
        audioProcessor.getTabEngine().deleteColumn(cursorColumn);
        // Keep cursor in valid position
        if (cursorColumn >= audioProcessor.getTabEngine().getNumColumns())
            tabEditor.setCursorPosition(audioProcessor.getTabEngine().getNumColumns() - 1, 0);
        tabEditor.repaint();
    }
}

void TabVSTAudioProcessorEditor::addBarLine()
{
    // Add a bar line after cursor position
    int cursorColumn = tabEditor.getCursorColumn();
    audioProcessor.getTabEngine().insertBarLine(cursorColumn + 1);
    tabEditor.setCursorPosition(cursorColumn + 1, 0);
    tabEditor.repaint();
}

void TabVSTAudioProcessorEditor::toggleShortcutsPanel()
{
    isShortcutsPanelVisible = !isShortcutsPanelVisible;

    if (isShortcutsPanelVisible)
    {
        if (!shortcutsPanel)
        {
            shortcutsPanel = std::make_unique<KeyboardShortcutsPanel>();
            addAndMakeVisible(shortcutsPanel.get());
        }
        shortcutsPanel->setVisible(true);
    }
    else
    {
        if (shortcutsPanel)
            shortcutsPanel->setVisible(false);
    }

    resized(); // Update layout
}

void TabVSTAudioProcessorEditor::toggleMode()
{
    isEditorMode = !isEditorMode;

    if (isEditorMode)
    {
        modeButton.setButtonText("View Tab");
        tabEditorViewport.setVisible(true);
        asciiView.setVisible(false);
        exportButton.setVisible(false); // Hide buttons in editor mode
        exportFileButton.setVisible(false);
        importFileButton.setVisible(false);

        // Enable all edit controls
        stringsSelector.setEnabled(true);
        keySelector.setEnabled(true);
        tuningTypeSelector.setEnabled(true);

        // Enable custom tuning controls if visible
        for (auto& selector : customStringSelectors)
            if (selector) selector->setEnabled(true);

        // Enable section controls
        addSectionButton.setEnabled(true);
        removeSectionButton.setEnabled(true);
        for (auto& button : sectionButtons)
            if (button) button->setEnabled(true);

        // Enable part controls
        addPartButton.setEnabled(true);
        removePartButton.setEnabled(true);
        for (auto& button : partButtons)
            if (button) button->setEnabled(true);
    }
    else
    {
        modeButton.setButtonText("Edit Tab");
        updateAsciiView();
        tabEditorViewport.setVisible(false);
        asciiView.setVisible(true);
        exportButton.setVisible(true); // Show buttons in view mode
        exportFileButton.setVisible(true);
        importFileButton.setVisible(true);

        // Disable all edit controls
        stringsSelector.setEnabled(false);
        keySelector.setEnabled(false);
        tuningTypeSelector.setEnabled(false);

        // Disable custom tuning controls
        for (auto& selector : customStringSelectors)
            if (selector) selector->setEnabled(false);

        // Disable section controls
        addSectionButton.setEnabled(false);
        removeSectionButton.setEnabled(false);
        for (auto& button : sectionButtons)
            if (button) button->setEnabled(false);

        // Disable part controls
        addPartButton.setEnabled(false);
        removePartButton.setEnabled(false);
        for (auto& button : partButtons)
            if (button) button->setEnabled(false);
    }

    resized();
}

void TabVSTAudioProcessorEditor::updateAsciiView()
{
    juce::String tabText = audioProcessor.getTabEngine().exportToText();
    asciiView.setText(tabText);
}

void TabVSTAudioProcessorEditor::updateSectionButtons()
{
    // Clear existing section buttons
    for (auto& button : sectionButtons)
    {
        if (button)
            removeChildComponent(button.get());
    }
    sectionButtons.clear();

    int numSections = audioProcessor.getTabEngine().getNumSections();
    int currentSection = audioProcessor.getTabEngine().getCurrentSection();

    // Create button for each section
    for (int i = 0; i < numSections; ++i)
    {
        // Create a custom button class that handles double-clicks and right-clicks
        class SectionButton : public juce::TextButton
        {
        public:
            std::function<void()> onDoubleClick;
            std::function<void()> onRightClick;

            void mouseDoubleClick(const juce::MouseEvent& event) override
            {
                if (onDoubleClick)
                    onDoubleClick();
                juce::TextButton::mouseDoubleClick(event);
            }

            void mouseDown(const juce::MouseEvent& event) override
            {
                if (event.mods.isPopupMenu() && onRightClick)
                {
                    onRightClick();
                }
                else
                {
                    juce::TextButton::mouseDown(event);
                }
            }
        };

        auto button = std::make_unique<SectionButton>();
        juce::String sectionName = audioProcessor.getTabEngine().getSectionName(i);
        button->setButtonText(sectionName);
        button->setClickingTogglesState(true);
        button->setRadioGroupId(1000); // Radio group for exclusive selection

        // Reverse selection colors - selected is lighter
        button->setColour(juce::TextButton::buttonColourId, juce::Colour(0xff3a3a3a)); // Normal (darker)
        button->setColour(juce::TextButton::buttonOnColourId, juce::Colour(0xff5a5a5a)); // Selected (lighter)

        if (i == currentSection)
            button->setToggleState(true, juce::dontSendNotification);

        int sectionIndex = i;
        button->onClick = [this, sectionIndex] { sectionButtonClicked(sectionIndex); };
        button->onDoubleClick = [this, sectionIndex] { sectionButtonDoubleClicked(sectionIndex); };
        button->onRightClick = [this, sectionIndex] { sectionButtonRightClicked(sectionIndex); };

        addAndMakeVisible(button.get());
        sectionButtons.push_back(std::move(button));
    }
}

void TabVSTAudioProcessorEditor::sectionButtonClicked(int sectionIndex)
{
    audioProcessor.getTabEngine().setCurrentSection(sectionIndex);
    updatePartButtons(); // Update part buttons when switching sections
    resized();
    tabEditor.repaint();
}

void TabVSTAudioProcessorEditor::sectionButtonDoubleClicked(int sectionIndex)
{
    renameSection(sectionIndex);
}

void TabVSTAudioProcessorEditor::sectionButtonRightClicked(int sectionIndex)
{
    juce::PopupMenu menu;
    menu.addItem(1, "Clear Section");
    menu.addItem(2, "Copy Section");
    menu.addItem(3, "Paste Section", sectionClipboard != nullptr);
    menu.addSeparator();
    menu.addItem(4, "Rename Section");

    menu.showMenuAsync(juce::PopupMenu::Options(), [this, sectionIndex](int result)
    {
        switch (result)
        {
            case 1: clearSection(sectionIndex); break;
            case 2: copySection(sectionIndex); break;
            case 3: pasteSection(sectionIndex); break;
            case 4: renameSection(sectionIndex); break;
        }
    });
}

void TabVSTAudioProcessorEditor::addSection()
{
    // Show input dialog for section name
    auto* w = new juce::AlertWindow("New Section", "Enter section name:", juce::MessageBoxIconType::QuestionIcon);
    w->addTextEditor("name", "New Section", "Section Name:");
    w->addButton("OK", 1, juce::KeyPress(juce::KeyPress::returnKey));
    w->addButton("Cancel", 0, juce::KeyPress(juce::KeyPress::escapeKey));

    w->enterModalState(true, juce::ModalCallbackFunction::create([this, w](int result)
    {
        if (result == 1)
        {
            juce::String sectionName = w->getTextEditorContents("name");
            if (sectionName.isEmpty())
                sectionName = "New Section";

            audioProcessor.getTabEngine().addSection(sectionName);
            updateSectionButtons();
            resized();
        }
        delete w;
    }), true);
}

void TabVSTAudioProcessorEditor::removeSection()
{
    int currentSection = audioProcessor.getTabEngine().getCurrentSection();
    int numSections = audioProcessor.getTabEngine().getNumSections();

    if (numSections <= 1)
    {
        juce::NativeMessageBox::showMessageBoxAsync(
            juce::MessageBoxIconType::WarningIcon,
            "Cannot Remove",
            "You must have at least one section.");
        return;
    }

    juce::String sectionName = audioProcessor.getTabEngine().getSectionName(currentSection);

    auto callback = juce::ModalCallbackFunction::create([this, currentSection](int result)
    {
        if (result == 1) // 1 = OK button
        {
            audioProcessor.getTabEngine().removeSection(currentSection);
            updateSectionButtons();
            resized();
            tabEditor.repaint();
        }
    });

    juce::NativeMessageBox::showOkCancelBox(
        juce::MessageBoxIconType::QuestionIcon,
        "Remove Section",
        "Are you sure you want to remove section \"" + sectionName + "\"?",
        this,
        callback);
}

void TabVSTAudioProcessorEditor::renameSection(int sectionIndex)
{
    juce::String currentName = audioProcessor.getTabEngine().getSectionName(sectionIndex);

    // Show input dialog for new section name
    auto* w = new juce::AlertWindow("Rename Section", "Enter new section name:", juce::MessageBoxIconType::QuestionIcon);
    w->addTextEditor("name", currentName, "Section Name:");
    w->addButton("OK", 1, juce::KeyPress(juce::KeyPress::returnKey));
    w->addButton("Cancel", 0, juce::KeyPress(juce::KeyPress::escapeKey));

    w->enterModalState(true, juce::ModalCallbackFunction::create([this, w, sectionIndex](int result)
    {
        if (result == 1)
        {
            juce::String newName = w->getTextEditorContents("name");
            if (newName.isNotEmpty())
            {
                audioProcessor.getTabEngine().renameSection(sectionIndex, newName);
                updateSectionButtons();
                resized();
            }
        }
        delete w;
    }), true);
}

void TabVSTAudioProcessorEditor::updatePartButtons()
{
    // Clear existing part buttons
    for (auto& button : partButtons)
    {
        if (button)
            removeChildComponent(button.get());
    }
    partButtons.clear();

    int numParts = audioProcessor.getTabEngine().getNumParts();
    int currentPart = audioProcessor.getTabEngine().getCurrentPart();

    // Create button for each part
    for (int i = 0; i < numParts; ++i)
    {
        // Create a custom button class that handles double-clicks and right-clicks
        class PartButton : public juce::TextButton
        {
        public:
            std::function<void()> onDoubleClick;
            std::function<void()> onRightClick;

            void mouseDoubleClick(const juce::MouseEvent& event) override
            {
                if (onDoubleClick)
                    onDoubleClick();
                juce::TextButton::mouseDoubleClick(event);
            }

            void mouseDown(const juce::MouseEvent& event) override
            {
                if (event.mods.isPopupMenu() && onRightClick)
                    onRightClick();
                else
                    juce::TextButton::mouseDown(event);
            }
        };

        auto button = std::make_unique<PartButton>();
        juce::String partName = audioProcessor.getTabEngine().getPartName(i);
        button->setButtonText(partName);
        button->setClickingTogglesState(true);
        button->setRadioGroupId(2000); // Different radio group from sections

        // Reverse selection colors - selected is lighter
        button->setColour(juce::TextButton::buttonColourId, juce::Colour(0xff3a3a3a)); // Normal (darker)
        button->setColour(juce::TextButton::buttonOnColourId, juce::Colour(0xff5a5a5a)); // Selected (lighter)

        if (i == currentPart)
            button->setToggleState(true, juce::dontSendNotification);

        int partIndex = i;
        button->onClick = [this, partIndex] { partButtonClicked(partIndex); };
        button->onDoubleClick = [this, partIndex] { partButtonDoubleClicked(partIndex); };
        button->onRightClick = [this, partIndex] { partButtonRightClicked(partIndex); };

        addAndMakeVisible(button.get());
        partButtons.push_back(std::move(button));
    }
}

void TabVSTAudioProcessorEditor::partButtonClicked(int partIndex)
{
    audioProcessor.getTabEngine().setCurrentPart(partIndex);
    tabEditor.repaint();
}

void TabVSTAudioProcessorEditor::partButtonDoubleClicked(int partIndex)
{
    renamePart(partIndex);
}

void TabVSTAudioProcessorEditor::addPart()
{
    // Show input dialog for part name
    auto* w = new juce::AlertWindow("New Part", "Enter part name:", juce::MessageBoxIconType::QuestionIcon);
    w->addTextEditor("name", "New Part", "Part Name:");
    w->addButton("OK", 1, juce::KeyPress(juce::KeyPress::returnKey));
    w->addButton("Cancel", 0, juce::KeyPress(juce::KeyPress::escapeKey));

    w->enterModalState(true, juce::ModalCallbackFunction::create([this, w](int result)
    {
        if (result == 1)
        {
            juce::String partName = w->getTextEditorContents("name");
            if (partName.isEmpty())
                partName = "New Part";

            audioProcessor.getTabEngine().addPart(partName);
            updatePartButtons();
            resized();
        }
        delete w;
    }), true);
}

void TabVSTAudioProcessorEditor::removePart()
{
    int currentPart = audioProcessor.getTabEngine().getCurrentPart();
    int numParts = audioProcessor.getTabEngine().getNumParts();

    if (numParts <= 1)
    {
        juce::NativeMessageBox::showMessageBoxAsync(
            juce::MessageBoxIconType::WarningIcon,
            "Cannot Remove",
            "You must have at least one part.");
        return;
    }

    juce::String partName = audioProcessor.getTabEngine().getPartName(currentPart);

    auto callback = juce::ModalCallbackFunction::create([this, currentPart](int result)
    {
        if (result == 1) // 1 = OK button
        {
            audioProcessor.getTabEngine().removePart(currentPart);
            updatePartButtons();
            resized();
            tabEditor.repaint();
        }
    });

    juce::NativeMessageBox::showOkCancelBox(
        juce::MessageBoxIconType::QuestionIcon,
        "Remove Part",
        "Are you sure you want to remove part \"" + partName + "\"?",
        this,
        callback);
}

void TabVSTAudioProcessorEditor::renamePart(int partIndex)
{
    juce::String currentName = audioProcessor.getTabEngine().getPartName(partIndex);

    // Show input dialog for new part name
    auto* w = new juce::AlertWindow("Rename Part", "Enter new part name:", juce::MessageBoxIconType::QuestionIcon);
    w->addTextEditor("name", currentName, "Part Name:");
    w->addButton("OK", 1, juce::KeyPress(juce::KeyPress::returnKey));
    w->addButton("Cancel", 0, juce::KeyPress(juce::KeyPress::escapeKey));

    w->enterModalState(true, juce::ModalCallbackFunction::create([this, w, partIndex](int result)
    {
        if (result == 1)
        {
            juce::String newName = w->getTextEditorContents("name");
            if (newName.isNotEmpty())
            {
                audioProcessor.getTabEngine().renamePart(partIndex, newName);
                updatePartButtons();
                resized();
            }
        }
        delete w;
    }), true);
}

void TabVSTAudioProcessorEditor::partButtonRightClicked(int partIndex)
{
    juce::PopupMenu menu;
    menu.addItem(1, "Clear Part");
    menu.addItem(2, "Copy Part");
    menu.addItem(3, "Paste Part", partClipboard != nullptr);
    menu.addSeparator();
    menu.addItem(4, "Add Column");
    menu.addItem(5, "Remove Column");
    menu.addItem(6, "Add Bar Line");
    menu.addSeparator();
    menu.addItem(7, "Rename Part");

    menu.showMenuAsync(juce::PopupMenu::Options(), [this, partIndex](int result)
    {
        switch (result)
        {
            case 1: clearPart(partIndex); break;
            case 2: copyPart(partIndex); break;
            case 3: pastePart(partIndex); break;
            case 4: addColumn(); break;
            case 5: removeColumn(); break;
            case 6: addBarLine(); break;
            case 7: renamePart(partIndex); break;
        }
    });
}

void TabVSTAudioProcessorEditor::clearPart(int partIndex)
{
    juce::String partName = audioProcessor.getTabEngine().getPartName(partIndex);

    auto callback = juce::ModalCallbackFunction::create([this, partIndex](int result)
    {
        if (result == 1) // 1 = OK button
        {
            audioProcessor.getTabEngine().clearPart(partIndex);
            tabEditor.repaint();
        }
    });

    juce::NativeMessageBox::showOkCancelBox(
        juce::MessageBoxIconType::QuestionIcon,
        "Clear Part",
        "Are you sure you want to clear all data in part \"" + partName + "\"?",
        this,
        callback);
}

void TabVSTAudioProcessorEditor::copyPart(int partIndex)
{
    partClipboard = audioProcessor.getTabEngine().copyPartToXML(partIndex);
    if (partClipboard)
    {
        juce::String partName = audioProcessor.getTabEngine().getPartName(partIndex);
        juce::NativeMessageBox::showMessageBoxAsync(
            juce::MessageBoxIconType::InfoIcon,
            "Part Copied",
            "Part \"" + partName + "\" has been copied to clipboard.");
    }
}

void TabVSTAudioProcessorEditor::pastePart(int partIndex)
{
    if (!partClipboard)
        return;

    juce::String partName = audioProcessor.getTabEngine().getPartName(partIndex);

    auto callback = juce::ModalCallbackFunction::create([this, partIndex](int result)
    {
        if (result == 1) // 1 = OK button
        {
            audioProcessor.getTabEngine().pastePartFromXML(partIndex, *partClipboard);
            updatePartButtons();
            resized();
            tabEditor.repaint();
        }
    });

    juce::NativeMessageBox::showOkCancelBox(
        juce::MessageBoxIconType::QuestionIcon,
        "Paste Part",
        "This will replace all data in part \"" + partName + "\". Continue?",
        this,
        callback);
}

void TabVSTAudioProcessorEditor::clearSection(int sectionIndex)
{
    juce::String sectionName = audioProcessor.getTabEngine().getSectionName(sectionIndex);

    auto callback = juce::ModalCallbackFunction::create([this, sectionIndex](int result)
    {
        if (result == 1) // 1 = OK button
        {
            audioProcessor.getTabEngine().clearSection(sectionIndex);
            updatePartButtons(); // Update if this is current section
            resized();
            tabEditor.repaint();
        }
    });

    juce::NativeMessageBox::showOkCancelBox(
        juce::MessageBoxIconType::QuestionIcon,
        "Clear Section",
        "Are you sure you want to clear all tabs in section \"" + sectionName + "\"?",
        this,
        callback);
}

void TabVSTAudioProcessorEditor::copySection(int sectionIndex)
{
    sectionClipboard = audioProcessor.getTabEngine().copySectionToXML(sectionIndex);

    if (sectionClipboard)
    {
        juce::String sectionName = audioProcessor.getTabEngine().getSectionName(sectionIndex);
        juce::NativeMessageBox::showMessageBoxAsync(
            juce::MessageBoxIconType::InfoIcon,
            "Section Copied",
            "Section \"" + sectionName + "\" copied to clipboard.");
    }
}

void TabVSTAudioProcessorEditor::pasteSection(int sectionIndex)
{
    if (!sectionClipboard)
        return;

    juce::String sectionName = audioProcessor.getTabEngine().getSectionName(sectionIndex);

    auto callback = juce::ModalCallbackFunction::create([this, sectionIndex](int result)
    {
        if (result == 1) // 1 = OK button
        {
            audioProcessor.getTabEngine().pasteSectionFromXML(sectionIndex, *sectionClipboard);
            updatePartButtons(); // Update if this is current section
            resized();
            tabEditor.repaint();
        }
    });

    juce::NativeMessageBox::showOkCancelBox(
        juce::MessageBoxIconType::QuestionIcon,
        "Paste Section",
        "Are you sure you want to paste clipboard content to section \"" + sectionName + "\"?\nThis will replace all existing tabs in this section.",
        this,
        callback);
}
