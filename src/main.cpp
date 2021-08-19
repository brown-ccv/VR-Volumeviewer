#include "../include/vrapp/VolumeVisualizationApp.h"

int main(int argc, char **argv) {
    
   if (argc >= 1) {
		if (strcmp(argv[1], "-t") == 0) {
		
        std::cout << "VR-Volumeviewer exists" << std::endl;
        return 0;

		}
	}

    // argc, argv need to be passed to the app so that MinVR can parse command
    // line args to see which config files to load.
	VolumeVisualizationApp app(argc, argv);

    // Does not return until the program shuts down.
    app.run();

    return 0;
}