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
	layeredImg.allocate(320, 240, OF_IMAGE_COLOR);
	capturedGray.allocate(320, 240);

	threshold = 20;
}

//--------------------------------------------------------------
void ofApp::update(){
	
	//---- openCV stuff ------------------------------------------------------
	vidGrabber.update();
	//bNewFrame = vidGrabber.isFrameNew();
	
	if (vidGrabber.isFrameNew()) {

		colorImg.setFromPixels(vidGrabber.getPixels());
		
		colorImg.setFromPixels(cropSrc(colorImg.getPixels()).getPixels());

		//colorImg.remap(100, 100);
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
		
		//use these FX when seperating blobs
		//grayDiff.blur(blur);
		//grayDiff.blurGaussian(gaussianBlur);

		/*for (int i = 0; i < dilateMultiple; i++) {
		grayDiff.dilate();
		}
		for (int i = 0; i < erodeMultiple; i++) {
		grayDiff.erode();
		}*/

		//would be great if this was a shader...
		capturedImg.setFromPixels(colorImg.getPixels());
		
		multipliedImg = multImage(grayDiff.getPixels(), capturedImg);

		multipliedImg = distributeBrightness(multipliedImg);


		//this creates the multilayered img
		layeredImg = layeringBrightness(multipliedImg); //was capturedImg

		if (getFrame) {
			getFrame = !getFrame;

			ofLogNotice("Got Frame!");
			//should we save this image for future reference?

			//this vector needs to be reset everytime
			allBlobs.clear();
			
			for (int lay = 0; lay < layers; lay++) {
				capturedImg = layerByLayer(multipliedImg, lay); //get that layer

				capturedGray = capturedImg;
				//find contour of single layer
				contourFinder.findContours(capturedGray, 20, (320 * 240) / 3, 1, true);

				if (contourFinder.nBlobs != 0) { //crucial - will get a vector out of range without it
					for (int i = 0; i < contourFinder.nBlobs; i++) { //add this in to create a vector containing points from all blobs
						layerBlobs.clear();
						for (int t = 0; t < contourFinder.blobs[i].nPts; t++) {
							layerBlobs.push_back(contourFinder.blobs[i].pts[t]);
							//only needed for scaling
							//layerBlobs[t][0] = ofMap(scaleData[t][0], 0.0, 320.0, 0.0, 1024.0, true);
							//layerBlobs[t][1] = ofMap(scaleData[t][1], 0.0, 240.0, 0.0, 768.0, true);
						}
						allBlobs.push_back(layerBlobs);
					}
				}
			}

		}

	}
}

//--------------------------------------------------------------
void ofApp::draw(){
	ofBackground(ofColor::red);

	if (pdfIt) {
		ofBeginSaveScreenAsPDF("layerImage" + ofToString(ofGetElapsedTimeMillis()) + ".pdf", false);
	}

	if (!mode) {

		ofPushMatrix();
		colorImg.draw(0.0, 0.0);
		ofDrawBitmapString("Original", colorImg.getWidth() / 2, colorImg.getHeight() + 30);
		ofPopMatrix();

		ofPushMatrix();
		ofTranslate(colorImg.getWidth(), 0);
		grayDiff.draw(0,0);
		ofDrawBitmapString("Absolute Difference", grayDiff.getWidth() / 2, grayDiff.getHeight() + 30);
		ofPopMatrix();

		ofPushMatrix();
		ofTranslate(colorImg.getWidth() + grayDiff.getWidth(), 0);
		multipliedImg.draw(0, 0);
		ofDrawBitmapString("Background Subtraction", multipliedImg.getWidth() / 2, multipliedImg.getHeight() + 30);
		ofPopMatrix();

		ofPushMatrix();
		ofTranslate(0.0, colorImg.getWidth());
		layeredImg.draw(0, 0);
		ofDrawBitmapString("Layered", capturedGray.getWidth() / 2, capturedGray.getHeight() + 30);
		ofPopMatrix();

		ofPushMatrix();
		ofTranslate(layeredImg.getWidth(), colorImg.getWidth());
		ofPushStyle();
		//drawPoints(allBlobs);
		drawContours(allBlobs);
		ofPopStyle();

		ofDrawBitmapString("Captured", capturedGray.getWidth() / 2, capturedGray.getHeight() + 30);	
		ofPopMatrix();

	}
	else {
		drawContours(allBlobs);
	}

	if (pdfIt) {
		ofEndSaveScreenAsPDF();
		pdfIt = false;
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
	case 'p':
		pdfIt = true;
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
	gui.add(layers.setup("layers", 5, 2, 9));
	gui.add(brMin.setup("Min", 0, 0, 150));
	gui.add(brMax.setup("Max", 255, 150, 255));
	gui.add(top.setup("top", 0.0, 0.0, 1.0));
	gui.add(bottom.setup("bottom", 1.0, 0.0, 1.0));
	gui.add(left.setup("left", 0.0, 0.0, 1.0));
	gui.add(right.setup("right", 1.0, 0.0, 1.0));



}

ofImage ofApp::layerByLayer(ofImage input, int lNum) {

	//input = distributeBrightness(input);

	ofColor newColor;

	int layAmt = 255 / layers;
	int imgX = input.getWidth();
	int imgY = input.getHeight();

	ofPixels newPix = input.getPixels();

	//set pixels of specific copy
	for (int d = 0; d < imgX; d++) {
		for (int c = 0; c < imgY; c++) {

			newColor = newPix.getColor(d, c);

			//color average method
			int brightness = (newColor.r + newColor.g + newColor.b) / 3.0;
			
			//if a given pixel falls in range set to white else set to black

			//may need to 
			if ((layAmt * lNum) - brightness < layAmt) {
				newColor.r = 255;
				newColor.g = 255;
				newColor.b = 255;
			}
			else {
				newColor.r = 0;
				newColor.g = 0;
				newColor.b = 0;
			}
			newPix.setColor(d, c, newColor);
		}
	}
	input.setFromPixels(newPix);
	return input;
}

ofImage ofApp::layeringBrightness(ofImage input) {
	
	input = distributeBrightness(input);

	ofColor newColor;

	int layAmt = 255 / layers;
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


void ofApp::drawPoints(vector<vector <ofPoint> > points) {

	int layAmt = 255 / layers;

	for (int i = 0; i<points.size(); i++) {
		//std::cout << "Draw these blobs: " + points.size() << "\n";
		ofSetColor(layAmt * i, layAmt * i, layAmt * i);
		for (int b = 0; b < points[i].size(); b++) {
			ofDrawEllipse(points[i][b], 2, 2);
		}
	}
}


vector<ofPolyline> ofApp::getContours(vector<vector <ofPoint> > points) {

	vector<ofPolyline> polylines;

	for (int i = 0; i<points.size(); i++) {
		if (points[i].size() != 0) {
			ofPolyline p;
			for (int b = 0; b < points[i].size(); b++) {
				//ofPoint aPoint = scalePoint(points[i][b]);
				ofPoint aPoint = points[i][b];
				p.addVertex(aPoint.x, aPoint.y);
			}
			p = p.getSmoothed(polylineSmoothSize, polylineSmoothShape);
			polylines.push_back(p);
		}
	}

	return polylines;
}

void ofApp::drawContours(vector<vector <ofPoint> > points) {

	int layAmt = 255 / layers;

	vector<ofPolyline> polylines = getContours(points);

	//for (auto & line : polylines) {
	for(int p = 0; p < polylines.size(); p++){
		//ofNoFill();
		ofFill();
		ofSetColor(p * layAmt);
		//line.draw();

		// draw filled polyline
		
		int offsetX = 320 * (p % 3);

		int offsetY = 240 * int (p / 3);

		if (polylines[p].getVertices().size() != 0) {

			ofBeginShape();
			if (!mode) {
				for (int i = 0; i < polylines[p].getVertices().size(); i++) {
					ofVertex(polylines[p].getVertices().at(i).x, polylines[p].getVertices().at(i).y);
				}
			}
			else {
				for (int i = 0; i < polylines[p].getVertices().size(); i++) {
					ofVertex(polylines[p].getVertices().at(i).x + offsetX, polylines[p].getVertices().at(i).y + offsetY);
				}
			}
			
			ofEndShape();
		}
	}
}

ofImage ofApp::cropSrc(ofImage input) {

	int wid = input.getWidth();
	int hig = input.getHeight();

	int cX1 = wid * left;
	int cX2 = wid * right;
	int cY1 = hig * top;
	int cY2 = hig * bottom;

	input.crop(cX1, cY1, cX2, cY2);

	input.resize(wid, hig);

	return input;
}
void ofApp::shadingFX(int fx) {
	//solid colors

	//half tone dots

	//hash marks
}