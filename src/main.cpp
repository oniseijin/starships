#include "ofMain.h"
#include "ofApp.h"


//========================================================================
int main( ){
    // Self-contained bundle: prefer embedded Resources/data over the
    // oF default (folder-containing-the-app/data) so the app runs from
    // /Applications without a sibling data dir.
#ifdef TARGET_OSX
    {
        auto resourcesData = ofFilePath::join(ofFilePath::getCurrentExeDir(), "../Resources/data/");
        ofDirectory d(resourcesData);
        if (d.exists()) ofSetDataPathRoot(resourcesData);
    }
#endif
    int w = MAX_WIDTH;
    int h = MAX_HEIGHT;
	ofSetupOpenGL(w,h,OF_WINDOW);			// <-------- setup the GL context

    // this kicks off the running of my app
	// can be OF_WINDOW or OF_FULLSCREEN
	// pass in width and height too:
	ofRunApp(new ofApp());

}
