#pragma once

#include "ofMain.h"
#include "ofxGui.h"
#include "ofxOpenCv.h"

class ofApp : public ofBaseApp{

	public:
		void setup();
		void update();
		void draw();

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

		ofVideoGrabber 		vidGrabber;

		ofxCvColorImage			colorImg;
		ofxCvGrayscaleImage 	grayImage;
		ofxCvGrayscaleImage 	grayBg;
		ofxCvGrayscaleImage 	grayDiff;
		ofxCvGrayscaleImage		tempBk;
		
		ofImage		capturedImg;


		ofxCvContourFinder 	contourFinder;

		int 				threshold;

		bool bHide = false; //hide the GUI
		void				setupGui();
		ofxPanel			gui;
		ofxToggle			mode;
		ofxToggle			flipHorizontal;
		ofxToggle			flipVertical;
		ofxToggle			invert;
		ofxIntSlider		gaussianBlur;
		ofxIntSlider		blur;
		ofxIntSlider		thresholdValue;
		ofxIntSlider		dilateMultiple;
		ofxIntSlider		erodeMultiple;
		ofxIntSlider		polylineSmoothSize;
		ofxIntSlider		minContour;
		ofxFloatSlider		polylineSmoothShape;
		ofxIntSlider		layers;
		ofxIntSlider		brMin;
		ofxIntSlider		brMax;


		ofImage layeringBrightness(ofImage input);
		ofImage distributeBrightness(ofImage input);

		ofImage multImage(ofImage input1, ofImage input2);

		ofImage multipliedImg;

		bool bNewFrame;
	
		bool bLearnBackground;

		bool bShowVideo = true;

		bool getFrame = false;
		
		vector <vector <ofPoint> > allBlobs;

};
