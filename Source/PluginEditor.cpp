/*
  ==============================================================================

    This file contains the basic framework code for a JUCE plugin editor.

  ==============================================================================
*/

#include "PluginProcessor.h"
#include "PluginEditor.h"

//==============================================================================
ResonatorProjectAudioProcessorEditor::ResonatorProjectAudioProcessorEditor (ResonatorProjectAudioProcessor& p)
    : AudioProcessorEditor (&p), audioProcessor (p)
{

    // Useful variables
	juce::Path path;
	path.addRectangle(50, 200, 50, 50);

#if(_DEBUG)
	bufferView = new juce::ShapeButton(
				juce::String("Pulse"), 
				juce::Colour(128, 0, 0), 
				juce::Colour(150, 0, 0), 
				juce::Colour(90, 0, 0)
		);
	componentView = new juce::ShapeButton(
				juce::String("Pulse"), 
				juce::Colour(0, 128, 0), 
				juce::Colour(0, 150, 0), 
				juce::Colour(0, 90, 0)
		);
	valueTreeView = new juce::ShapeButton(
				juce::String("Pulse"), 
				juce::Colour(0, 0, 128), 
				juce::Colour(0, 0, 150), 
				juce::Colour(0, 0, 90)
		);
	fontAndColourView = new juce::ShapeButton(
				juce::String("Pulse"), 
				juce::Colour(128, 0, 128), 
				juce::Colour(150, 0, 150), 
				juce::Colour(90, 0, 90)
		);

	bufferView->setShape(path, false, false, false);
	bufferView->onClick = [this] { audioProcessor.toggleBufferDebugger(); };
	addAndMakeVisible(bufferView);

	componentView->setShape(path, false, false, false);
	componentView->onClick = [this] { toggleComponentDebugger(); };
	addAndMakeVisible(componentView);
	componentDebuggerOn = false;

	valueTreeView->setShape(path, false, false, false);
	valueTreeView->onClick = [this] { openValueTreeDebugger(); };
	addAndMakeVisible(valueTreeView);

	fontAndColourView->setShape(path, false, false, false);
	fontAndColourView->onClick = [this] { toggleFontAndColourDesigner(); };
	addAndMakeVisible(fontAndColourView);
	fontAndColourDesignerOn = false;
#endif

    // Resonator controls
    for (int i = 0; i < NUM_RESONATORS; i++) {
		// Set parameters for resonator frequency knobs
		resonatorFrequency[i].setSliderStyle(juce::Slider::RotaryHorizontalVerticalDrag);
		resonatorFrequency[i].setRange(20.0, 10000.0, 0.7);
		resonatorFrequency[i].setSkewFactor(.5);
		resonatorFrequency[i].setTextBoxStyle(juce::Slider::NoTextBox, false, 90, 0);
		resonatorFrequency[i].setPopupDisplayEnabled(true, false, this);
		resonatorFrequency[i].setTextValueSuffix(" Hz");
		resonatorFrequency[i].onValueChange = [this, i] { (&audioProcessor)->setFrequency(i, resonatorFrequency[i].getValue()); };
		resonatorFrequency[i].setValue(440.0);
		
		resonatorNumberLabel[i].setText(std::to_string(i+1), juce::NotificationType::dontSendNotification);
		resonatorNumberLabel[i].attachToComponent(&resonatorFrequency[i], false);

		// Set parameters for resonator feedback knobs
		resonatorDecay[i].setSliderStyle(juce::Slider::RotaryHorizontalVerticalDrag);
		resonatorDecay[i].setRange(0., 1., 0.01);
		//resonatorFeedback[i].setSkewFactorFromMidPoint(.9);
		resonatorDecay[i].setTextBoxStyle(juce::Slider::NoTextBox, false, 90, 0);
		resonatorDecay[i].setPopupDisplayEnabled(true, false, this);
		resonatorDecay[i].onValueChange = [this, i] { (&audioProcessor)->setDecay(i, resonatorDecay[i].getValue() / 2.0); };
		resonatorDecay[i].setValue(0.5);

		// Set parameters for pulse button
		pulseButtons[i] = new juce::ShapeButton(
				juce::String("Pulse"), 
				juce::Colour(128, 128, 128), 
				juce::Colour(150, 150, 150), 
				juce::Colour(90, 90, 90)
		);
		pulseButtons[i]->setShape(path, false, false, false);
		pulseButtons[i]->onClick = [this, i] { audioProcessor.addNoise(i); };

		// Set parameters for volume knob
		volumeSlider[i].setSliderStyle(juce::Slider::RotaryHorizontalVerticalDrag);
		volumeSlider[i].setRange(0.0, 10.0, 0.01);
		volumeSlider[i].setTextBoxStyle(juce::Slider::NoTextBox, false, 90, 0);
		volumeSlider[i].setPopupDisplayEnabled(true, false, this);
		volumeSlider[i].onValueChange = [this, i] { (&audioProcessor)->setVolume(volumeSlider[i].getValue() / 10.); };
		volumeSlider[i].setValue(10.0);

		addAndMakeVisible(&resonatorFrequency[i]);
		addAndMakeVisible(&resonatorDecay[i]);
		addAndMakeVisible(&volumeSlider[i]);
		addAndMakeVisible(pulseButtons[i]);
    }

    // Set parameters for output volume knob
    outputVolumeSlider.setSliderStyle(juce::Slider::RotaryHorizontalVerticalDrag);
    outputVolumeSlider.setRange(-100.0, 12.0, 0.1);
    outputVolumeSlider.setSkewFactorFromMidPoint(0.0);
    outputVolumeSlider.setTextBoxStyle(juce::Slider::NoTextBox, false, 90, 0);
    outputVolumeSlider.setPopupDisplayEnabled(true, false, this);
    outputVolumeSlider.onValueChange = [this] { (&audioProcessor)->setOutputVolume(juce::Decibels::decibelsToGain(outputVolumeSlider.getValue())); };
    outputVolumeSlider.setValue(0.0);

    outputVolumeLabel.setText("Output", juce::NotificationType::dontSendNotification);
    outputVolumeLabel.attachToComponent(&outputVolumeSlider, false);
    addAndMakeVisible(&outputVolumeSlider);

    // Set parameters for labels
	resonatorFrequencyLabel.setText("Frequency", juce::NotificationType::dontSendNotification);
	resonatorFrequencyLabel.attachToComponent(&resonatorFrequency[0], true);

    resonatorDecayLabel.setText("Decay", juce::NotificationType::dontSendNotification);
    resonatorDecayLabel.attachToComponent(&resonatorDecay[0], true);
    
	volumeSliderLabel.setText("Volume", juce::NotificationType::dontSendNotification);
	volumeSliderLabel.attachToComponent(&volumeSlider[0], true);

    // Make sure that before the constructor has finished, you've set the
    // editor's size to whatever you need it to be.
#if(_DEBUG)
	setSize(850, 600);
#endif
#if(_RELEASE)
    setSize (800, 600);
#endif
}

ResonatorProjectAudioProcessorEditor::~ResonatorProjectAudioProcessorEditor()
{
	for (int i = 0; i < NUM_RESONATORS; i++) delete pulseButtons[i];
}

//==============================================================================
void ResonatorProjectAudioProcessorEditor::paint (juce::Graphics& g)
{
    // (Our component is opaque, so we must completely fill the background with a solid colour)
    g.fillAll (getLookAndFeel().findColour (juce::ResizableWindow::backgroundColourId));

    g.setColour (juce::Colours::white);
    g.setFont (15.0f);
    g.drawFittedText("Resonators", 0, 0, getWidth(), 30, juce::Justification::centred, 1);
}

void ResonatorProjectAudioProcessorEditor::resized()
{
    // This is generally where you'll want to lay out the positions of any
    // subcomponents in your editor..
#if(_DEBUG)
	bufferView->setBounds(800, 0, 50, 50);
	componentView->setBounds(800, 50, 50, 50);
	valueTreeView->setBounds(800, 100, 50, 50);
	fontAndColourView->setBounds(800, 150, 50, 50);
    int knobWidth = (getWidth()-150) / NUM_RESONATORS * 0.8;
    int knobPadding = knobWidth * 0.25;
#endif
#if(_RELEASE)
    int knobWidth = (getWidth()-100) / NUM_RESONATORS * 0.8;
    int knobPadding = knobWidth * 0.25;
#endif
	for (int i = 0; i < NUM_RESONATORS; i++) {
        resonatorFrequency[i].setBounds(100 + knobPadding*(i + 1) + knobWidth*i, 50, knobWidth, knobWidth);
		resonatorDecay[i].setBounds(100 + knobPadding*(i + 1) + knobWidth*i, 50 + knobWidth + knobPadding, knobWidth, knobWidth);
		volumeSlider[i].setBounds(100 + knobPadding*(i + 1) + knobWidth*i, 50 + 2*(knobWidth + knobPadding), knobWidth, knobWidth);
		pulseButtons[i]->setBounds(100 + knobPadding*(i + 1) + knobWidth*i, 50 + 3*(knobWidth + knobPadding), knobWidth, knobWidth);
	}
    outputVolumeSlider.setBounds(650, 450, 100, 100);
}

#if(_DEBUG)
void ResonatorProjectAudioProcessorEditor::toggleComponentDebugger()
{
    if (!componentDebuggerOn) componentDebugger = new jcf::ComponentDebugger(this);
    else delete componentDebugger;
    componentDebuggerOn = !componentDebuggerOn;
}

void ResonatorProjectAudioProcessorEditor::openValueTreeDebugger()
{
    valueTreeDebugger = new jcf::ValueTreeDebugger();
}

void ResonatorProjectAudioProcessorEditor::toggleFontAndColourDesigner()
{
	/*
    if (!fontAndColourDesignerOn) fontAndColourDesigner = new jcf::FontAndColourDesigner();
    else delete fontAndColourDesigner;
    fontAndColourDesignerOn = !fontAndColourDesignerOn;
	*/
}
#endif
