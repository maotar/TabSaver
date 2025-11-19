#pragma once

#include <JuceHeader.h>

class KeyboardShortcutsPanel : public juce::Component
{
public:
    KeyboardShortcutsPanel();
    ~KeyboardShortcutsPanel() override;

    void paint(juce::Graphics& g) override;
    void resized() override;
    void mouseDown(const juce::MouseEvent& event) override;

private:
    struct ShortcutInfo
    {
        juce::String key;
        juce::String description;
    };

    std::vector<ShortcutInfo> notationShortcuts;
    std::vector<ShortcutInfo> editingShortcuts;
    juce::Rectangle<int> coffeeButtonBounds;

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(KeyboardShortcutsPanel)
};
