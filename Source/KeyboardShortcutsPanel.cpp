#include "KeyboardShortcutsPanel.h"

KeyboardShortcutsPanel::KeyboardShortcutsPanel()
{
    // Notation shortcuts
    notationShortcuts = {
        {"0-24", "Set fret number"},
        {"h", "Hammer-on"},
        {"p", "Pull-off"},
        {"/", "Slide up"},
        {"\\", "Slide down"},
        {"b", "Bend"},
        {"r", "Release bend"},
        {"t", "Tap"},
        {"x", "Mute"},
        {"~", "Vibrato"},
        {"< or >", "Harmonic"}
    };

    // Editing shortcuts
    editingShortcuts = {
        {"Arrow Keys", "Navigate"},
        {"Delete/Backspace", "Clear fret"},
        {"[", "Remove column"},
        {"]", "Add column"},
        {"|", "Add bar line"}
    };

    setSize(280, 435);
}

KeyboardShortcutsPanel::~KeyboardShortcutsPanel()
{
}

void KeyboardShortcutsPanel::paint(juce::Graphics& g)
{
    // Background
    g.setColour(juce::Colour(0xff2a2a2a));
    g.fillRoundedRectangle(getLocalBounds().toFloat(), 4.0f);

    // Border
    g.setColour(juce::Colour(0xff404040));
    g.drawRoundedRectangle(getLocalBounds().toFloat().reduced(0.5f), 4.0f, 1.0f);

    auto bounds = getLocalBounds().reduced(15, 10);
    int yPos = bounds.getY();

    // Title
    g.setColour(juce::Colours::white);
    g.setFont(juce::FontOptions(14.0f, juce::Font::bold));
    g.drawText("Keyboard Shortcuts", bounds.getX(), yPos, bounds.getWidth(), 20, juce::Justification::centredLeft);
    yPos += 30;

    // Notation section
    g.setColour(juce::Colour(0xff90CAF9));
    g.setFont(juce::FontOptions(12.0f, juce::Font::bold));
    g.drawText("NOTATION", bounds.getX(), yPos, bounds.getWidth(), 16, juce::Justification::centredLeft);
    yPos += 20;

    g.setFont(juce::FontOptions(11.0f));
    for (const auto& shortcut : notationShortcuts)
    {
        // Key
        g.setColour(juce::Colours::lightgrey);
        g.drawText(shortcut.key, bounds.getX(), yPos, 80, 16, juce::Justification::centredLeft);

        // Description
        g.setColour(juce::Colours::grey);
        g.drawText(shortcut.description, bounds.getX() + 85, yPos, bounds.getWidth() - 85, 16, juce::Justification::centredLeft);

        yPos += 18;
    }

    yPos += 8;

    // Editing section
    g.setColour(juce::Colour(0xffA5D6A7));
    g.setFont(juce::FontOptions(12.0f, juce::Font::bold));
    g.drawText("EDITING", bounds.getX(), yPos, bounds.getWidth(), 16, juce::Justification::centredLeft);
    yPos += 20;

    g.setFont(juce::FontOptions(11.0f));
    for (const auto& shortcut : editingShortcuts)
    {
        // Key
        g.setColour(juce::Colours::lightgrey);
        g.drawText(shortcut.key, bounds.getX(), yPos, 80, 16, juce::Justification::centredLeft);

        // Description
        g.setColour(juce::Colours::grey);
        g.drawText(shortcut.description, bounds.getX() + 85, yPos, bounds.getWidth() - 85, 16, juce::Justification::centredLeft);

        yPos += 18;
    }

    yPos += 20;

    // Buy me a coffee link
    int buttonWidth = 130;
    int buttonHeight = 28;
    int buttonX = bounds.getX() + (bounds.getWidth() - buttonWidth) / 2;
    coffeeButtonBounds = juce::Rectangle<int>(buttonX, yPos, buttonWidth, buttonHeight);

    // Draw coffee button background
    g.setColour(juce::Colour(0xffD4A574)); // Subdued coffee brown color
    g.fillRoundedRectangle(coffeeButtonBounds.toFloat(), 4.0f);

    // Draw coffee button text
    g.setColour(juce::Colours::black);
    g.setFont(juce::FontOptions(11.0f, juce::Font::bold));
    g.drawText("Buy me a coffee", coffeeButtonBounds, juce::Justification::centred);
}

void KeyboardShortcutsPanel::resized()
{
}

void KeyboardShortcutsPanel::mouseDown(const juce::MouseEvent& event)
{
    if (coffeeButtonBounds.contains(event.getPosition()))
    {
        juce::URL url("https://buymeacoffee.com/maotar");
        url.launchInDefaultBrowser();
    }
}
