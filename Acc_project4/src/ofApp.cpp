#include "ofApp.h"

//--------------------------------------------------------------
void ofApp::setup(){
	ofSetFrameRate(60);

	setupGui();

	vidGrabber.setVerbose(true);
	vidGrabber.setDeviceID(0);

	vidGrabber.initGrabber(320, 240);
	//vidGrabber.videoSettings();

	//must allocate pixels before reading anything in to them
	colorImg.allocate(320, 240);
	grayImage.allocate(320, 240);
	grayBg.allocate(320, 240);
	grayDiff.allocate(320, 240);
	capturedImg.allocate(320, 240, OF_IMAGE_COLOR);
	multipliedImg.allocate(320, 240, OF_IMAGE_COLOR);

	threshold = 20;
}

//--------------------------------------------------------------
void ofApp::update(){
	
	//---- openCV stuff ------------------------------------------------------
	vidGrabber.update();
	//bNewFrame = vidGrabber.isFrameNew();
	
	if (vidGrabber.isFrameNew()) {
		colorImg.setFromPixels(vidGrabber.getPixels());
		
		//process input image - precapture

		if (flipHorizontal && flipVertical) {
			colorImg.mirror(1, 1);
		}
		else if (flipHorizontal) {
			colorImg.mirror(0, 1);
		}
		else if (flipVertical) {
			colorImg.mirror(1, 0);
		}

		if (invert) {
			colorImg.invert();
		}
		
		grayImage = colorImg;

		if (bLearnBackground) {
			grayBg = grayImage;
			bLearnBackground = false;
		}

		//remove background
		grayDiff.absDiff(grayBg, grayImage);
		grayDiff.threshold(thresholdValue);
		
		use these FX when seperating blobs
		//grayDiff.blur(blur);
		//grayDiff.blurGaussian(gaussianBlur);

		//would be great if this was a shader...
		capturedImg.setFromPixels(colorImg.getPixels());
		capturedImg = layeringBrightness(capturedImg); //was capturedImg

		multipliedImg = multImage(grayDiff.getPixels(), capturedImg);

		//getFrame = !getFrame;

		

		//should we save this image for future reference?


		/*for (int i = 0; i < dilateMultiple; i++) {
			grayDiff.dilate();
		}
		for (int i = 0; i < erodeMultiple; i++) {
			grayDiff.erode();
		}*/
		
		contourFinder.findContours(grayDiff, 20, (320 * 240) / 3, 1, false);

		//this vector needs to be reset everytime
		allBlobs.clear();

		/*
		if (contourFinder.nBlobs != 0) { //crucial - will get a vector out of range without it
										 //std::cout << "Original amount of blobs: " << contourFinder.nBlobs << "\n";
			for (int i = 0; i < contourFinder.nBlobs; i++) { //add this in to create a vector containing points from all blobs
				scaleData.clear();
				//posData.clear();
				for (int t = 0; t < contourFinder.blobs[i].nPts; t++) {
					//posData.push_back(contourFinder.blobs[i].pts[t]);
					scaleData.push_back(contourFinder.blobs[i].pts[t]);
					scaleData[t][0] = ofMap(scaleData[t][0], 0.0, 320.0, 0.0, 1024.0, true);
					scaleData[t][1] = ofMap(scaleData[t][1], 0.0, 240.0, 0.0, 768.0, true);
				}
				allBlobs.push_back(scaleData);
			}
		}*/
	}
}

//--------------------------------------------------------------
void ofApp::draw(){
	ofBackgroundGradient(ofColor::white, ofColor::gray);

	if (!mode) {
		// draw the original local video image
		ofSetHexColor(0xffffff);
		ofPushMatrix();
		//ofTranslate(ofGetWidth() / 2 - colorImg.getWidth() / 2, ofGetHeight() / 2 - colorImg.getHeight() / 2);
		if (bShowVideo) {
			colorImg.draw(0.0, 0.0);
			grayDiff.draw(0, colorImg.getWidth());
			capturedImg.draw(colorImg.getWidth(), 0.0);
			multipliedImg.draw(grayDiff.getWidth(), colorImg.getWidth());
		}
		ofPopMatrix();
	}
	else {
		
	}
	
	// Draw GUI
	if (!bHide) {
		gui.draw();
	}
}

//--------------------------------------------------------------
void ofApp::keyPressed(int key){

}

//--------------------------------------------------------------
void ofApp::keyReleased(int key){
	switch(key) {
	case 's': 
		getFrame = true;
		break;
	case ' ':
		bLearnBackground = true;
		break;
	}
}

//--------------------------------------------------------------
void ofApp::mouseMoved(int x, int y ){

}

//--------------------------------------------------------------
void ofApp::mouseDragged(int x, int y, int button){

}

//--------------------------------------------------------------
void ofApp::mousePressed(int x, int y, int button){

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


void ofApp::setupGui() {

	gui.setup();
	gui.add(mode.setup("Capture/ Process", false));
	gui.add(flipHorizontal.setup("flip horizontally", false));
	gui.add(flipVertical.setup("flip vertically", false));
	gui.add(invert.setup("invert", false));
	gui.add(gaussianBlur.setup("gaussian blur", 1, 1, 20));
	gui.add(blur.setup("blur", 1, 1, 20));
	gui.add(thresholdValue.setup("threshold value", 100, 1, 255));
	gui.add(dilateMultiple.setup("dilation multiple", 0, 0, 10));
	gui.add(erodeMultiple.setup("erosion multiple", 0, 0, 10));
	gui.add(polylineSmoothShape.setup("polyline smooth shape", 0.0, 0.0, 1.0));
	gui.add(polylineSmoothSize.setup("polyline smooth size", 0, 0, 32));
	gui.add(minContour.setup("minimum contour size", 10000, 0.0, (640 * 360) / 3));
	gui.add(layers.setup("layers", 5, 2, 10));
	gui.add(brMin.setup("Min", 0, 0, 150));
	gui.add(brMax.setup("Max", 255, 150, 255));


}


ofImage ofApp::layeringBrightness(ofImage input) {
	
	input = distributeBrightness(input);

	ofColor newColor;

	int layAmt = 255 / layers;
	ofLogNotice(ofToString(layAmt));
	int imgX = input.getWidth();
	int imgY = input.getHeight();

	ofPixels newPix = input.getPixels();

	//set pixels of specific copy
	for (int d = 0; d < imgX; d++) {
		for (int c = 0; c < imgY; c++) {

			newColor = newPix.getColor(d, c);

			//color average method
			int brightness = (newColor.r + newColor.g + newColor.b) / 3.0;

			for (int l = 1; l < layers + 1; l++) { 
				if ((layAmt * l) - brightness < layAmt) {
					newColor.r = layAmt * l;
					newColor.g = layAmt * l;
					newColor.b = layAmt * l;
				}
			}
			newPix.setColor(d, c, newColor);
		}
	}
	input.setFromPixels(newPix);
	return input;	
}

/*
stretch values to full color spectrum + clamp*/

ofImage ofApp::distributeBrightness(ofImage input) {

	ofColor disColor;

	int imgX = input.getWidth();
	int imgY = input.getHeight();

	for (int d = 0; d < imgX; d++) {
		for (int c = 0; c < imgY; c++) {
			
			disColor = input.getColor(d, c);

			//maybe remove the ofClamp
			disColor.r = ofClamp(ofMap(disColor.r, brMin, brMax, 0, 255), 0, 255);
			disColor.g = ofClamp(ofMap(disColor.g, brMin, brMax, 0, 255), 0, 255);
			disColor.b = ofClamp(ofMap(disColor.b, brMin, brMax, 0, 255), 0, 255);

			input.setColor(d, c, disColor);

		}
	}
	return input;
}


ofImage ofApp::multImage(ofImage input1, ofImage input2) {

	ofColor color1;
	ofColor color2;

	int imgX = input1.getWidth();
	int imgY = input1.getHeight();

	for (int d = 0; d < imgX; d++) {
		for (int c = 0; c < imgY; c++) {

			color1 = input1.getColor(d, c);
			color2 = input2.getColor(d, c);

			int brightness = (color1.r + color1.g + color1.b) / 3.0;

			if (brightness == 255) {
				color1.r = color2.r;
				color1.g = color2.g;
				color1.b = color2.b;

				input1.setColor(d, c, color1);
			}
		}
	}
	return input1;
}