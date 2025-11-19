#include "TabEditorComponent.h"

TabEditorComponent::TabEditorComponent(TabEngine& engine)
    : tabEngine(engine),
      currentColumn(0),
      currentString(0),
      cellWidth(40),
      cellHeight(30),
      stringNameWidth(50),
      lastInputTime(0),
      pendingTechnique(Technique::None)
{
    tabEngine.addListener(this);
    setWantsKeyboardFocus(true);
    updateSize();
}

TabEditorComponent::~TabEditorComponent()
{
    tabEngine.removeListener(this);
}

void TabEditorComponent::paint(juce::Graphics& g)
{
    g.fillAll(juce::Colour(0xff1e1e1e)); // Dark background

    drawGrid(g);
    drawCursor(g);
}

void TabEditorComponent::drawGrid(juce::Graphics& g)
{
    auto tuning = tabEngine.getCurrentTuning();
    int numStrings = tabEngine.getNumStrings();
    int numColumns = tabEngine.getNumColumns();

    // Draw string names
    g.setColour(juce::Colours::lightgrey);
    g.setFont(14.0f);

    for (int str = 0; str < numStrings; ++str)
    {
        int y = str * cellHeight + 20;
        g.drawText(tuning.notes[numStrings - 1 - str],
                   5, y, stringNameWidth - 10, cellHeight,
                   juce::Justification::centredRight);
    }

    // Draw grid lines
    g.setColour(juce::Colour(0xff404040));

    // Horizontal lines (strings)
    for (int str = 0; str <= numStrings; ++str)
    {
        int y = str * cellHeight + 20;
        g.drawLine((float)stringNameWidth, (float)y,
                   (float)(stringNameWidth + numColumns * cellWidth), (float)y);
    }

    // Vertical lines (columns)
    for (int col = 0; col <= numColumns; ++col)
    {
        int x = stringNameWidth + col * cellWidth;

        // Draw bar lines thicker
        if (col < numColumns && tabEngine.isColumnBarLine(col))
        {
            g.setColour(juce::Colours::lightgrey);
            g.drawLine((float)x, 20.0f,
                       (float)x, (float)(20 + numStrings * cellHeight), 3.0f);
            g.setColour(juce::Colour(0xff404040));
        }
        else
        {
            g.drawLine((float)x, 20.0f,
                       (float)x, (float)(20 + numStrings * cellHeight));
        }
    }

    // Draw fret numbers
    g.setColour(juce::Colours::white);
    g.setFont(16.0f);

    for (int col = 0; col < numColumns; ++col)
    {
        for (int str = 0; str < numStrings; ++str)
        {
            int fret = tabEngine.getFret(col, numStrings - 1 - str);
            if (fret >= 0)
            {
                int x = stringNameWidth + col * cellWidth;
                int y = str * cellHeight + 20;

                // Get technique and build display string
                Technique tech = tabEngine.getTechnique(col, numStrings - 1 - str);
                bool beforeFret = tabEngine.isTechniqueBeforeFret(col, numStrings - 1 - str);
                juce::String displayText = juce::String(fret);
                juce::String techStr = "";

                // Get technique string
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
                    displayText = "x";
                }
                else if (tech == Technique::Harmonic)
                {
                    displayText = "<" + displayText + ">";
                }
                else if (!techStr.isEmpty())
                {
                    // Apply technique before or after based on flag
                    if (beforeFret)
                        displayText = techStr + displayText;
                    else
                        displayText = displayText + techStr;
                }

                g.drawText(displayText,
                           x, y, cellWidth, cellHeight,
                           juce::Justification::centred);
            }
        }
    }
}

void TabEditorComponent::drawCursor(juce::Graphics& g)
{
    int numStrings = tabEngine.getNumStrings();

    // Convert cursor position to screen coordinates
    int visualString = numStrings - 1 - currentString;
    int x = stringNameWidth + currentColumn * cellWidth;
    int y = visualString * cellHeight + 20;

    // Draw cursor border
    g.setColour(juce::Colours::yellow);
    g.drawRect(x + 2, y + 2, cellWidth - 4, cellHeight - 4, 2);
}

void TabEditorComponent::resized()
{
    // Could add scrolling here if tab extends beyond visible area
}

bool TabEditorComponent::keyPressed(const juce::KeyPress& key)
{
    // Number keys (0-9) for fret numbers
    if (key.getTextCharacter() >= '0' && key.getTextCharacter() <= '9')
    {
        int digit = key.getTextCharacter() - '0';
        enterDigit(digit);
        return true;
    }

    // Technique keys (h, p, /, \, b, r, t, x, ~, <, >)
    char c = key.getTextCharacter();
    if (c == 'h' || c == 'p' || c == '/' || c == '\\' ||
        c == 'b' || c == 'r' || c == 't' || c == 'x' ||
        c == '~' || c == '<' || c == '>')
    {
        enterTechniqueChar(c);
        return true;
    }

    // Arrow keys for navigation
    if (key.isKeyCode(juce::KeyPress::leftKey))
    {
        moveCursor(-1, 0);
        return true;
    }
    if (key.isKeyCode(juce::KeyPress::rightKey))
    {
        moveCursor(1, 0);
        return true;
    }
    if (key.isKeyCode(juce::KeyPress::upKey))
    {
        moveCursor(0, 1);
        return true;
    }
    if (key.isKeyCode(juce::KeyPress::downKey))
    {
        moveCursor(0, -1);
        return true;
    }

    // Delete/Backspace to clear fret
    if (key.isKeyCode(juce::KeyPress::deleteKey) ||
        key.isKeyCode(juce::KeyPress::backspaceKey))
    {
        deleteFret();
        return true;
    }

    // '[' to remove column at cursor
    if (c == '[')
    {
        int currentColumns = tabEngine.getNumColumns();
        if (currentColumns > 1 && currentColumn >= 0 && currentColumn < currentColumns)
        {
            tabEngine.deleteColumn(currentColumn);
            // Keep cursor in valid position
            if (currentColumn >= tabEngine.getNumColumns())
                currentColumn = tabEngine.getNumColumns() - 1;
            repaint();
        }
        return true;
    }

    // ']' to add column after cursor
    if (c == ']')
    {
        // Insert after current cursor position
        tabEngine.insertColumn(currentColumn + 1);
        // Move cursor to the new column
        currentColumn++;
        repaint();
        return true;
    }

    // '|' to add bar line after cursor
    if (c == '|')
    {
        // Insert bar line after current cursor position
        tabEngine.insertBarLine(currentColumn + 1);
        // Move cursor to the new bar line column
        currentColumn++;
        repaint();
        return true;
    }

    return false;
}

void TabEditorComponent::mouseDown(const juce::MouseEvent& event)
{
    int column, string;
    if (getCellAtPosition(event.x, event.y, column, string))
    {
        setCursorPosition(column, string);
        grabKeyboardFocus();
    }
}

void TabEditorComponent::tabDataChanged()
{
    updateSize();
    repaint();
}

void TabEditorComponent::updateSize()
{
    int numColumns = tabEngine.getNumColumns();
    int numStrings = tabEngine.getNumStrings();

    int width = stringNameWidth + numColumns * cellWidth + 20;
    int height = numStrings * cellHeight + 40;

    setSize(width, height);
}

void TabEditorComponent::moveCursor(int deltaColumn, int deltaString)
{
    // Clear any pending input when moving to a new cell
    pendingFretInput = "";
    pendingTechnique = Technique::None;

    currentColumn = juce::jlimit(0, tabEngine.getNumColumns() - 1, currentColumn + deltaColumn);
    currentString = juce::jlimit(0, tabEngine.getNumStrings() - 1, currentString + deltaString);
    repaint();
}

void TabEditorComponent::setCursorPosition(int column, int string)
{
    // Clear any pending input when moving to a new cell
    pendingFretInput = "";
    pendingTechnique = Technique::None;

    currentColumn = juce::jlimit(0, tabEngine.getNumColumns() - 1, column);
    currentString = juce::jlimit(0, tabEngine.getNumStrings() - 1, string);
    repaint();
}

void TabEditorComponent::getCellBounds(int column, int string, juce::Rectangle<int>& bounds) const
{
    int numStrings = tabEngine.getNumStrings();
    int visualString = numStrings - 1 - string;

    bounds.setX(stringNameWidth + column * cellWidth);
    bounds.setY(visualString * cellHeight + 20);
    bounds.setWidth(cellWidth);
    bounds.setHeight(cellHeight);
}

bool TabEditorComponent::getCellAtPosition(int x, int y, int& column, int& string) const
{
    if (x < stringNameWidth || y < 20)
        return false;

    int numStrings = tabEngine.getNumStrings();

    column = (x - stringNameWidth) / cellWidth;
    int visualString = (y - 20) / cellHeight;
    string = numStrings - 1 - visualString;

    if (column >= 0 && column < tabEngine.getNumColumns() &&
        string >= 0 && string < numStrings)
    {
        return true;
    }

    return false;
}

void TabEditorComponent::enterDigit(int digit)
{
    juce::uint32 currentTime = juce::Time::getMillisecondCounter();
    const juce::uint32 digitTimeout = 800; // 800ms delay to combine digits

    // Check if there's a pending technique entered before any digits
    // Once we determine this, we keep it throughout the multi-digit entry
    bool techniqueBeforeFret = false;

    // Check if we should start a new number or continue building the current one
    if (pendingFretInput.isEmpty() || (currentTime - lastInputTime) > digitTimeout)
    {
        // Start new number - check if technique was entered before this first digit
        techniqueBeforeFret = (pendingTechnique != Technique::None);
        pendingFretInput = juce::String(digit);
    }
    else
    {
        // Append to existing number - check if technique was set before the first digit
        // by checking if there's a fret already and what its technique position is
        int currentFret = tabEngine.getFret(currentColumn, currentString);
        if (currentFret >= 0)
        {
            techniqueBeforeFret = tabEngine.isTechniqueBeforeFret(currentColumn, currentString);
        }
        pendingFretInput += juce::String(digit);
    }

    lastInputTime = currentTime;

    // Parse the pending input
    int fretValue = pendingFretInput.getIntValue();

    // If it's a valid fret number (0-24), display it immediately
    if (fretValue >= 0 && fretValue <= 24)
    {
        tabEngine.setFret(currentColumn, currentString, fretValue);
        tabEngine.setTechnique(currentColumn, currentString, pendingTechnique, techniqueBeforeFret);
        repaint();

        // If we have 2 digits or the value is > 2 (can't go higher), clear pending input
        // but don't auto-advance
        if (pendingFretInput.length() >= 2 || fretValue > 2)
        {
            pendingFretInput = "";
            pendingTechnique = Technique::None;
        }
    }
    else
    {
        // Invalid fret number, just clear pending
        pendingFretInput = "";
        pendingTechnique = Technique::None;
    }
}

void TabEditorComponent::enterTechniqueChar(char c)
{
    // Map character to technique
    Technique tech = Technique::None;
    if (c == 'h') tech = Technique::HammerOn;
    else if (c == 'p') tech = Technique::PullOff;
    else if (c == '/') tech = Technique::SlideUp;
    else if (c == '\\') tech = Technique::SlideDown;
    else if (c == 'b') tech = Technique::Bend;
    else if (c == 'r') tech = Technique::ReleaseBend;
    else if (c == 't') tech = Technique::Tap;
    else if (c == 'x') tech = Technique::Mute;
    else if (c == '~') tech = Technique::Vibrato;
    else if (c == '<' || c == '>') tech = Technique::Harmonic;

    int currentFret = tabEngine.getFret(currentColumn, currentString);

    // Special handling for mute - can be entered on empty cells
    if (tech == Technique::Mute)
    {
        tabEngine.setFret(currentColumn, currentString, 0); // Use fret 0 as placeholder
        tabEngine.setTechnique(currentColumn, currentString, tech);
        pendingFretInput = "";
        pendingTechnique = Technique::None;
        repaint();
        return;
    }

    // Store pending technique for later application
    pendingTechnique = tech;
    lastInputTime = juce::Time::getMillisecondCounter();

    // If there's already a fret, apply the technique immediately AFTER the fret
    if (currentFret >= 0)
    {
        tabEngine.setTechnique(currentColumn, currentString, tech, false); // false = after fret
        repaint();
    }
    // Otherwise, the technique will be applied BEFORE when the next digit is entered
}

void TabEditorComponent::commitPendingFret()
{
    // Just clear the pending input without moving
    pendingFretInput = "";
    pendingTechnique = Technique::None;
}

void TabEditorComponent::enterFret(int fret)
{
    tabEngine.setFret(currentColumn, currentString, fret);
    moveCursor(1, 0); // Auto-advance to next column
}

void TabEditorComponent::deleteFret()
{
    tabEngine.setFret(currentColumn, currentString, -1);
}

void TabEditorComponent::insertColumn()
{
    tabEngine.insertColumn(currentColumn);
}

void TabEditorComponent::deleteColumn()
{
    tabEngine.deleteColumn(currentColumn);
}
