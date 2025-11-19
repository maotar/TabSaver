#pragma once

#include <JuceHeader.h>
#include "TabEngine.h"

//==============================================================================
class TabEditorComponent : public juce::Component,
                           public TabEngine::Listener
{
public:
    TabEditorComponent(TabEngine& engine);
    ~TabEditorComponent() override;

    void paint (juce::Graphics&) override;
    void resized() override;

    // Keyboard input
    bool keyPressed(const juce::KeyPress& key) override;
    void mouseDown(const juce::MouseEvent& event) override;

    // TabEngine::Listener
    void tabDataChanged() override;

    // Navigation
    void moveCursor(int deltaColumn, int deltaString);
    void setCursorPosition(int column, int string);
    int getCursorColumn() const { return currentColumn; }

    // Size calculation
    void updateSize();

private:
    TabEngine& tabEngine;

    // Cursor position
    int currentColumn;
    int currentString;

    // Display settings
    int cellWidth;
    int cellHeight;
    int stringNameWidth;

    // Multi-digit fret input
    juce::String pendingFretInput;
    juce::uint32 lastInputTime;
    Technique pendingTechnique;

    // Helper methods
    void drawGrid(juce::Graphics& g);
    void drawCursor(juce::Graphics& g);
    void getCellBounds(int column, int string, juce::Rectangle<int>& bounds) const;
    bool getCellAtPosition(int x, int y, int& column, int& string) const;

    void enterFret(int fret);
    void enterDigit(int digit);
    void enterTechniqueChar(char c);
    void commitPendingFret();
    void deleteFret();
    void insertColumn();
    void deleteColumn();

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (TabEditorComponent)
};
