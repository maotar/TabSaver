TabSaver - Installation Instructions
=====================================

Thank you for downloading TabSaver!

MACOS INSTALLATION (EASY METHOD)
---------------------------------

1. Extract the downloaded archive
2. Open Terminal (you can find it in Applications > Utilities)
3. Type: cd
   (note: there's a space after 'cd')
4. Drag the extracted TabSaver folder into the Terminal window
5. Press Enter
6. Type: ./install-macos.sh
7. Press Enter
8. Restart your DAW

The plugins will be installed to:
- VST3: ~/Library/Audio/Plug-Ins/VST3/
- AU: ~/Library/Audio/Plug-Ins/Components/


MACOS INSTALLATION (MANUAL METHOD)
-----------------------------------

If you prefer to install manually or the script doesn't work:

1. Extract the downloaded archive
2. Remove the quarantine flag by running in Terminal:

   xattr -cr /path/to/TabSaver.vst3
   xattr -cr /path/to/TabSaver.component

   (Replace /path/to/ with the actual path to the files)

3. Copy the plugins to:

   TabSaver.vst3 → ~/Library/Audio/Plug-Ins/VST3/
   TabSaver.component → ~/Library/Audio/Plug-Ins/Components/

4. Restart your DAW


WINDOWS INSTALLATION
--------------------

1. Extract the downloaded archive
2. Copy TabSaver.vst3 to: C:\Program Files\Common Files\VST3\
3. Restart your DAW


LINUX INSTALLATION
------------------

1. Extract the downloaded archive
2. Copy TabSaver.vst3 to: ~/.vst3/
3. Restart your DAW


TROUBLESHOOTING
---------------

If macOS shows "TabSaver is damaged and can't be opened":
- This is a security warning for unsigned apps
- Use the install-macos.sh script, which handles this automatically
- Or manually run: xattr -cr /path/to/plugin

If the plugin doesn't appear in your DAW:
- Make sure you've restarted your DAW after installation
- Check your DAW's plugin scan/rescan function
- Verify the plugin is in the correct folder

For more help, visit: https://github.com/maotar/TabSaver


USAGE
-----

1. Load TabSaver in your DAW as a VST3 or AU plugin
2. Select your guitar tuning from the dropdown
3. Click in the editor and use keyboard shortcuts to write tabs:
   - Numbers 0-9: Enter fret numbers
   - Arrow keys: Navigate the grid
   - Space: Move to next column
   - Delete/Backspace: Clear a fret
   - +/=: Insert column
   - Cmd+-: Delete column

Your tabs are automatically saved with your DAW project!

Use the Export button to save tabs as .tab files for sharing or transferring
between projects.


Enjoy TabSaver!
