#pragma once

#include "ofMain.h"
#include "ofxMidi.h"
#include "ofxMaxim.h"
#include "ofxGui.h"
#include "SequenceButton.h"

#define NButtons 16
#define NUMCHANNELS 5

class ofApp : public ofBaseApp, public ofxMidiListener {

	public:
		void setup();
		void update();
		void draw();
		//-------------------MAXIM---------------------------------//
		void audioOut(float * output, int bufferSize, int nChannels);
		//---------------------------------------------------------//
		void drawWaveform();
		//-------------------MIDI----------------------------------//
		void newMidiMessage(ofxMidiMessage &message);
		void midiDebug();
		void exit();
		//-------------------GUI----------------------------------//

		void keyPressed(int key);
		void keyReleased(int key);
		void mouseMoved(int x, int y );
		void mouseDragged(int x, int y, int button);
		void mousePressed(int x, int y, int button);
		void mouseReleased(int x, int y, int button);
		void mouseEntered(int x, int y);
		void mouseExited(int x, int y);
		void windowResized(int w, int h);
		void dragEvent(ofDragInfo dragInfo);
		void gotMessage(ofMessage msg);

		//---------------------------------------------------------//
		//--SETUP-SOUND--//
		ofSoundStream soundStream;
		unsigned bufferSize, sampleRate;
		double currentSample;
		double mix_0, mix_1, mix_2;
		double masterMix[2], outputs_0[2], outputs_1[2], outputs_2[2];
		maxiMix channelMix[NUMCHANNELS];

		//-Audio-Analysis
		ofxMaxiFFT fft;
		ofxMaxiFFTOctaveAnalyzer oct;

		//---------------------------------------------------------//
		//--SETUP-MIDI--//
		ofxMidiIn midi;
		vector <ofxMidiMessage> messages;
		vector <int> midiNotes;
		maxiClock clock;
		convert mtof;				   //convert midi notes to frequency
		int midiTrigger, midiNote;

		int n;
		int seqPitch[16];
		int playHead;
		int position;
		int seqTrigger[NUMCHANNELS];
		int seq_0[16] = { 1,0,0,1, 1,0,0,1, 1,0,1,0, 1,0,0,1 };
		int seq_1[16] = { 1,1,0,1, 1,1,0,1, 1,0,1,0, 1,0,1,1 };
		int seq_2[16];

		//---------------------------------------------------------//
		//Sequence Buttons
		SequenceButton seqButtons_0[NButtons], seqButtons_1[NButtons], seqButtons_2[NButtons];
		
		//---------------------------------------------------------//
		//--SYNTHESIS--//
		maxiOsc osc[NUMCHANNELS];
		maxiOsc lfo[NUMCHANNELS];
		maxiEnv env[NUMCHANNELS];
		maxiFilter filter[NUMCHANNELS];
		maxiDelayline delay[NUMCHANNELS];
		maxiKick kick;

		double oscOut[NUMCHANNELS];
		double lfoOut[NUMCHANNELS];
		double envOut[NUMCHANNELS];
		double pitch[NUMCHANNELS];
		double gain[NUMCHANNELS];
		double delayOut[NUMCHANNELS];
		double filterOut[NUMCHANNELS];
	
		//---------------------------------------------------------//
		//--GUI--//
		ofxPanel gui;
		ofParameter<float> masterGainSlider;
		ofxVec4Slider adsrSlider;

		//---------------------------------------------------------//
		//--DRAW-WAVEFROM--//
		vector <float> volHistory;
		float smoothedVol;
		float scaledVol;
		float soundBuffer[512];


};
