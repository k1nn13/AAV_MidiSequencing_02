#include "SequenceButton.h"


SequenceButton::SequenceButton() {

}

void SequenceButton::setup(float _x, float _y, float _size) {

	x = _x;
	y = _y;
	size = _size;

	buttonSelect.x = x;
	buttonSelect.y = y;
	buttonSelect.width = size;
	buttonSelect.height = size;

}

void SequenceButton::update() {
	collision = buttonSelect.inside(ofGetMouseX() + buttonSelect.width / 2, ofGetMouseY() + buttonSelect.height / 2);
}

void SequenceButton::draw() {

	ofPushStyle();
	ofSetRectMode(OF_RECTMODE_CENTER);

	if (collision) {
		ofNoFill();
		ofSetColor(255, 0, 0);
	}
	else {
		ofNoFill();
		ofSetColor(255, 255, 0);
	}

	if (seqValue) {
		ofFill();
		ofSetColor(255, 0, 0);
	}

	ofDrawRectRounded(buttonSelect, 10);

	ofPopStyle();
}


void SequenceButton::mousePressed() {
	if (collision) {
		seqValue += 1;

		if (seqValue > 1) {
			seqValue = 0;
		}
	}
}

