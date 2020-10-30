#include "saam.hpp"
#include "optparser.hpp"
#include <cstdio>
#include <climits>

using namespace Saam;
using namespace Tuiapp;

namespace {
	struct RuntimeConfig {
		bool monochrome = 0;
		bool grayscale = 0;
		bool reverse = 0;
		bool shading = 0;
		bool noskip = 0;
		int density = 50;
		unsigned char shadingThreshold = 1;
		bool loop = 0;
        bool autoscale = 0;
		int maxWidth = INT_MAX, maxHeight = INT_MAX;
		const char* media = nullptr;
		int coefficient = 603;
		char** fileList = nullptr;
		size_t fileCount;
		int interval = 6000000;

		RuntimeConfig(Cmdline&);
		~RuntimeConfig();
	};

    void help(const char* appname) {
		const char* helpmsg = R"(Simple Ascii Art Maker, showing one or more image files at a time.

Usage: %s [options]... [parameters]=[value]... [file]...
If the first file name given starts with ":", the file will be used as a list
of images and images will be loaded according to the list.

Color option:
  (unspecified)         display in color
  --monochrome          display in black and while
  --grayscale           display in grayscale

Graphics mode option:
  (unspecified)         use only characters for graphics
  --reverse             same as the first, but with a light color background
  --shading             use characters with background colors for graphics
                        (only works without --monochrome)

Other display options:
  --loop                after all images shown, continue from the beginning
  --noskip              wait for terminal to respond every frame

Parameters:
  --max-width           specify the maximum width of graphics in characters.
                        Any image larger than this size would be scaled to fit
  --max-height          maximum height of graphics, like above
  --density             density of characters in the graphic, between [0, 100]
  --shading-threshold   degree of the difference between background color and
                        characters, between [0, 255].
  --coefficient         color conversion model, only for grayscale graphics;
                        '603' for Rec.603, '709' for Rec.709, etc.
  --media               media for playing in the background while showing the
                        images
  --interval            time interval between showing different images, in
                        milliseconds

General option:
  --help                display this help and exit
)";
		fprintf(stderr, helpmsg, appname);
    }
}

int main(int argc, char** argv) {
    Cmdline cmd(argc, argv);
    RuntimeConfig config(cmd);
    





    
    return 0;
}

namespace {
	RuntimeConfig::RuntimeConfig(Cmdline& cmd) {

	}

	RuntimeConfig::~RuntimeConfig() {

	}
}