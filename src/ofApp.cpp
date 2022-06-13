#include "ofApp.h"

//--------------------------------------------------------------
void ofApp::setup(){

	//-----OF-SETUP-------//
	ofSetFrameRate(60);
	ofSetVerticalSync(true);
	ofEnableAlphaBlending();
	ofEnableSmoothing();


	gui.setup();
	gui.add(masterGainSlider.set("master gain", 0.4, 0.01, 1));
	gui.add(adsrSlider.setup("ADSR", ofVec4f(10, 5, 5, 1000), ofVec4f(0, 0, 0, 0), ofVec4f(5000, 5000, 5000, 20000)));

	//--MIDI-SETUP--//
	midi.listInPorts();			  // display midi instrument name in terminal
	midi.openPort("ReMOTE SL 0"); // ***select midi port string or int***
	midi.addListener(this);
	clock.setTicksPerBeat(4);		
	clock.setTempo(100);

	//---MAXIM-SETUP---//
	fft.setup(1024, 512, 256);
	oct.setup(44100, 1024, 10);
	sampleRate = 44100;
	bufferSize = 512;

	//--SoundStream-Settings--//
	ofSoundStreamListDevices();
	ofSoundStreamSettings settings;
	settings.setApi(ofSoundDevice::MS_ASIO);  //select sound card

	ofSoundStreamSetup(2, 2, this, sampleRate, bufferSize, 4);
	ofxMaxiSettings::setup(sampleRate, 2, bufferSize);
	//=======================================================//

}


//--------------------------------------------------------------
void ofApp::update(){


	//--ENVELOPES--//
	env[0].setAttack(  adsrSlider->x );
	env[0].setDecay(   adsrSlider->y );
	env[0].setSustain( adsrSlider->z );
	env[0].setRelease( adsrSlider->w );

	env[1].setAttack(1000);
	env[1].setDecay(500);
	env[1].setSustain(500);
	env[1].setRelease(2500);


	//--SETUP-UPDATE-SEQUENCE-BUTTONS--//
	for (int i = 0; i < NButtons; i++) {
		float w = ofGetWidth();
		float h = ofGetHeight();

		seqButtons_0[i].setup( (w*.3) + (55*i), h*.4, 50 );
		seqButtons_0[i].update();
		seq_0[i] = seqButtons_0[i].seqValue;

		seqButtons_1[i].setup((w*.3) + (55 * i), h*.5, 50);
		seqButtons_1[i].update();
		seq_1[i] = seqButtons_1[i].seqValue;

		seqButtons_2[i].setup((w*.3) + (55 * i), h*.6, 50);
		seqButtons_2[i].update();
		seq_2[i] = seqButtons_2[i].seqValue;
	}


}

//--------------------------------------------------------------
void ofApp::draw(){

	ofBackground(0);

	ofPushStyle();
	ofSetColor(255, 255, 0);
	ofDrawBitmapString("Bass", ofGetWidth() * .2, ofGetHeight() * 0.4);
	ofDrawBitmapString("Lead", ofGetWidth() * .2, ofGetHeight() * 0.5);
	ofDrawBitmapString("Kick", ofGetWidth() * .2, ofGetHeight() * 0.6);
	ofPopStyle();

	midiDebug();
	drawWaveform();

	// draw the sequence buttons
	for (int i = 0; i < NButtons; i++) {
		seqButtons_0[i].draw();
		seqButtons_1[i].draw();
		seqButtons_2[i].draw();
	}

	gui.draw();
	
}

//-----------------------AUDIO----------------------------------
void ofApp::audioOut(float * output, int bufferSize, int nChannels) {

	for (int i = 0; i < bufferSize; i++) {

		//----------------------------------------------
		clock.ticker();     //start clock
		if (clock.tick) {   //iterate through sequences
			seqTrigger[0] = seq_0[playHead % 16];
			seqTrigger[1] = seq_1[playHead % 16];
			seqTrigger[2] = seq_2[playHead % 16];

			if (seqTrigger[0] == 1) {
				//midiNotes[position];
				position++;
				if (position > midiNotes.size()-1) {
					position = 0;
				}
			}
			playHead++; //inrement playhead to select next note or trigger
		}

		//----------------------------------------------
		//create and array for the sequencer
		n = midiNotes.size();
		for (int i = 0; i < n; i++) {
			seqPitch[i] = midiNotes[i];
		}

		//---------SYNTHESIS-------//
		//pitch[0] = mtof.mtof(midiNote);							// convert midi note to frequency
		pitch[0] = mtof.mtof(seqPitch[position]);
		pitch[1] = mtof.mtof(seqPitch[position] + 36);
		pitch[2] = mtof.mtof(seqPitch[position] + 41);
		pitch[3] = mtof.mtof(seqPitch[position] + 48);

		//---------Voice-1-Poly-Bass---------//				
		envOut[0] = env[0].adsr(1., seqTrigger[0]);					// input midi to trigger envelope
		gain[0] = 0.1;												// set gain on osc

		lfoOut[0] = lfo[0].sinebuf(0.2);
		oscOut[0] = osc[0].pulse(pitch[0], 0.2);

		oscOut[1] = (osc[1].pulse(pitch[0], 0.6) + oscOut[0]) * envOut[0];
		filterOut[0] = filter[0].lores(oscOut[1], 100 + envOut[0] *500, envOut[0] * 2) * gain[0];

		mix_0 = filterOut[0];
		channelMix[0].stereo(mix_0, outputs_0, 0.5);

		//---------Voice-2-Strings---------//	
		envOut[1] = env[1].adsr(1., seqTrigger[1]);					// input midi to trigger envelope
		gain[1] = 0.1;

		lfoOut[1] = lfo[1].sinewave(envOut[1]);

		oscOut[2] = (osc[2].saw(pitch[1] + lfoOut[1]) * envOut[1]) * 0.08;
		oscOut[3] = (osc[3].saw(pitch[2] + lfoOut[1] * 2) * envOut[1]) * 0.08;
		oscOut[4] = (osc[4].saw(pitch[3] + lfoOut[1] * 4) * envOut[1]) * 0.08;

		filterOut[1] = filter[1].hires(oscOut[2], 500 + envOut[1] * 10, 0);
		filterOut[2] = filter[2].hires(oscOut[3], 500 + envOut[1] * 10, 0);
		filterOut[3] = filter[3].lores(filterOut[1] + filterOut[2] + oscOut[4], 1500 - envOut[1] *1000, 2);

		mix_1 = filterOut[3];
		channelMix[1].stereo(mix_1, outputs_1, 0.5);


		//setup kick drum
		kick.setPitch(55);
		kick.useFilter = true;
		kick.cutoff = 50;
		kick.setRelease(10);
		mix_2 = kick.play() * 0.6;
		channelMix[2].stereo(mix_2, outputs_2, 0.5);


		if (seqTrigger[2] == 1) {
			kick.trigger();
		}

		//=======Master=Stereo=Mix==========//

		masterMix[0] = (outputs_0[0] + outputs_1[0] + outputs_2[0]);			//add stereo outputs to master mix
		masterMix[1] = (outputs_0[1] + outputs_1[1] + outputs_2[1]);
		//==========MASTER-OUT==============//
		output[ i*nChannels     ] = masterMix[0] * masterGainSlider;
		output[ i*nChannels + 1 ] = masterMix[1] * masterGainSlider;
		//==================================//

		//-----------PROCESS-FFT------------//
		double fftMix = (masterMix[0] + masterMix[1]) *.5;
		if (fft.process(fftMix)) {
			oct.calculate(fft.magnitudes);
		}
		//----------------------------------//
	}
}

//--------------------------------------------------------------
void ofApp::drawWaveform() {

	ofNoFill();	
	ofSetLineWidth(1);
	ofBeginShape();
	for (int i = 0; i < oct.nAverages; i++) {

		ofColor colour;
		colour.setHsb((int)ofMap(i, 0, oct.nAverages, 1, 255), 255, 255);
		ofSetColor(colour);

		float size = 5 + oct.averages[i];
		int x = (int)((ofGetWidth() / 2) * i / oct.nAverages) + ofGetWidth() / 4;
		float y = ofGetHeight() * .2;
		float fftHeight = fft.magnitudes[i];

		ofVertex(x, y + fftHeight * -2);			//draw a line of fft magnitude
		ofVertex(x, y + fft.magnitudes[i] * 2);
		ofDrawRectangle(x, y, 1, + fftHeight * -2);	//draw rectangles with fft magnitude
		ofDrawRectangle(x, y, 1, +fftHeight * 2);	//draw rectangles with fft magnitude
	}
	ofEndShape(false);
}

//-------------------------MIDI---------------------------------
void ofApp::newMidiMessage(ofxMidiMessage &message) {
	
	//-------------------------------//
	if (message.status == MIDI_NOTE_ON) {          // setup midi to trigger note 
		midiNote = message.pitch;
		midiTrigger = 1;

		midiNotes.push_back(midiNote);
		if (midiNotes.size() > 16) {
			midiNotes.erase(midiNotes.begin());
		}

	}
	else if (message.status == MIDI_NOTE_OFF) {
		midiTrigger = 0;
	}
	                     // setup midi to trigger pitch
	//-------------------------------//
												   // (this could be used to create a sequence later)
	messages.push_back(message);                   // create an array of midi messages and set size
	if (messages.size() > 16) {
		messages.erase(messages.begin());
	}

}

//--------------------------------------------------------------
void ofApp::midiDebug() {

	//--------------------------------------------// print midi note on/off and pitch to screen
	for (int i = 0; i < messages.size(); i++) { 
		ofxMidiMessage &message = messages[i];
		string midiTriggerDebug;
		if (message.status == MIDI_NOTE_ON) {
			midiTriggerDebug = "On";
		}
		else if (message.status == MIDI_NOTE_OFF) {
			midiTriggerDebug = "Off";
		}
													// draw midi notes to the screen
		//ofSetColor(255);
		//ofDrawBitmapString("Note " + midiTriggerDebug + ": Pitch: " + ofToString(message.pitch), ofGetWidth() * .8, 128 + i * 16);
	}


	for (int i = 0; i < midiNotes.size(); i++) {
		ofSetColor(255, 255, 0);
		ofDrawBitmapString("Midi Note: " + ofToString(seqPitch[i]), ofGetWidth() * 0.8, ofGetHeight() * 0.04 + i * 16);
	}

}

//--------------------------------------------------------------
void ofApp::keyPressed(int key){

}

//--------------------------------------------------------------
void ofApp::keyReleased(int key){

}

//--------------------------------------------------------------
void ofApp::mouseMoved(int x, int y ){

}

//--------------------------------------------------------------
void ofApp::mouseDragged(int x, int y, int button){

}

//--------------------------------------------------------------
void ofApp::mousePressed(int x, int y, int button){
	for (int i = 0; i < NButtons; i++) {
		seqButtons_0[i].mousePressed();
		seqButtons_1[i].mousePressed();
		seqButtons_2[i].mousePressed();
	}
	
}

//--------------------------------------------------------------
void ofApp::mouseReleased(int x, int y, int button){

}

//--------------------------------------------------------------
void ofApp::mouseEntered(int x, int y){

}

//--------------------------------------------------------------
void ofApp::mouseExited(int x, int y){

}

//--------------------------------------------------------------
void ofApp::windowResized(int w, int h){

}

//--------------------------------------------------------------
void ofApp::gotMessage(ofMessage msg){

}

//--------------------------------------------------------------
void ofApp::dragEvent(ofDragInfo dragInfo){ 

}

//--------------------------------------------------------------
void ofApp::exit() {
	
	midi.closePort();
	midi.removeListener(this);
	ofSoundStreamClose();
}
