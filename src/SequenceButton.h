#pragma once
#include "ofMain.h"

class SequenceButton
{
	public:
		SequenceButton();
		void setup(float _x, float _y, float _size);
		void update();
		void draw();
		void mousePressed();

		int seqValue;
		bool collision;
		ofRectangle buttonSelect;

	private:
		float x;
		float y;
		float size;

};

/*
for (int i = 0; i < 10; i++) {
	Mover mv = Mover(ofVec3f(100, 200), ofVec3f(0, 0), ofVec3f(0, 0), 2.0);
	movers.push_back(mv);
}
I’ve declared the movers vector in ofApp.h:

std::vector<Mover> movers;

*/