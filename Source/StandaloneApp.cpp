#include <JuceHeader.h>
#include "PluginProcessor.h"

//==============================================================================
class StandalonePluginHolder  : public juce::AudioProcessorPlayer
{
public:
    StandalonePluginHolder()
    {
        processor = std::make_unique<TabVSTAudioProcessor>();
        setupAudioDevices();
    }

    ~StandalonePluginHolder() override
    {
        deletePlugin();
        shutDownAudioDevices();
    }

    void setupAudioDevices()
    {
        audioDeviceManager.initialiseWithDefaultDevices(2, 2);
        AudioProcessorPlayer::setProcessor(processor.get());
        audioDeviceManager.addAudioCallback(this);
    }

    void shutDownAudioDevices()
    {
        audioDeviceManager.removeAudioCallback(this);
        audioDeviceManager.closeAudioDevice();
    }

    void deletePlugin()
    {
        AudioProcessorPlayer::setProcessor(nullptr);
        processor = nullptr;
    }

    TabVSTAudioProcessor* getProcessor() const { return processor.get(); }

private:
    std::unique_ptr<TabVSTAudioProcessor> processor;
    juce::AudioDeviceManager audioDeviceManager;
};

//==============================================================================
class StandaloneWindow : public juce::DocumentWindow
{
public:
    StandaloneWindow()
        : DocumentWindow("TabSaver",
                        juce::Desktop::getInstance().getDefaultLookAndFeel()
                            .findColour(juce::ResizableWindow::backgroundColourId),
                        juce::DocumentWindow::allButtons)
    {
        pluginHolder = std::make_unique<StandalonePluginHolder>();

        auto* processor = pluginHolder->getProcessor();
        if (processor != nullptr)
        {
            auto* editor = processor->createEditorIfNeeded();
            if (editor != nullptr)
            {
                setContentNonOwned(editor, true);
                setUsingNativeTitleBar(true);  // Enable native titlebar with fullscreen button
                centreWithSize(getWidth(), getHeight());
                setVisible(true);
            }
        }
    }

    ~StandaloneWindow() override
    {
        if (auto* processor = pluginHolder->getProcessor())
            processor->editorBeingDeleted(dynamic_cast<juce::AudioProcessorEditor*>(getContentComponent()));

        clearContentComponent();
        pluginHolder = nullptr;
    }

    void closeButtonPressed() override
    {
        juce::JUCEApplicationBase::quit();
    }

private:
    std::unique_ptr<StandalonePluginHolder> pluginHolder;

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(StandaloneWindow)
};

//==============================================================================
class TabVSTApplication : public juce::JUCEApplication
{
public:
    TabVSTApplication() = default;

    const juce::String getApplicationName() override { return "TabSaver"; }
    const juce::String getApplicationVersion() override { return "1.0.0"; }
    bool moreThanOneInstanceAllowed() override { return true; }

    void initialise(const juce::String&) override
    {
        mainWindow = std::make_unique<StandaloneWindow>();
    }

    void shutdown() override
    {
        mainWindow = nullptr;
    }

    void systemRequestedQuit() override
    {
        quit();
    }

private:
    std::unique_ptr<StandaloneWindow> mainWindow;
};

//==============================================================================
START_JUCE_APPLICATION(TabVSTApplication)
