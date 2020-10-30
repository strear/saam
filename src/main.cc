#include "saam.hpp"
#include "optparser.hpp"
#include <cstdio>
#include <climits>
#include <functional>

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

		RuntimeConfig(Cmdline&, std::function<void(bool, const char*, const char*)>&);
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

	auto errchk = [&](bool flag, const char* errtype, const char* detail) {
		if (flag) return;

		fprintf(stderr, "%s: %s", cmd.appname());
		if (detail != nullptr) fprintf(stderr, " -- '%s'", detail);
		fprintf(stderr, "\nTry '--help' for more information.\n");

		quit(errtype[0]);
	};

	RuntimeConfig config(cmd, errchk);







	return 0;
}

namespace {
	RuntimeConfig::RuntimeConfig(Cmdline& cmd,
		std::function<void(bool, const char*, const char*)>& errchk) {

		if (cmd.get("help")) {
			help(cmd.appname());
			quit(0);
		}

		monochrome = cmd.get("monochrome");
		reverse = cmd.get("reverse");

		if (!monochrome) {
			grayscale = cmd.get("grayscale");
			if (!reverse) shading = cmd.get("shading");

			if (shading) {
				const char* thresholdStr;
				if (cmd.get("shadingThreshold", thresholdStr)) {
					errchk(parseInt(thresholdStr, this->shadingThreshold),
						"invalid threshold value", thresholdStr);
					errchk(shadingThreshold > 0 && shadingThreshold <= 255,
						"threshold value out of range", thresholdStr);
				}
			}
		}
		loop = cmd.get("loop");
		noskip = cmd.get("noskip");
		autoscale = cmd.get("autoscale");

		const char* maxWidthStr;
		if (cmd.get("max-width", maxWidthStr)) {
			errchk(parseInt(maxWidthStr, maxWidth),
				"invalid maximum width value", maxWidthStr);
		}

		const char* maxHeightStr;
		if (cmd.get("max-height", maxHeightStr)) {
			errchk(parseInt(maxHeightStr, maxHeight),
				"invalid maximum height value", maxHeightStr);
		}

		const char* coefficientStr;
		if (cmd.get("coefficient", coefficientStr)) {
			errchk(parseInt(coefficientStr, coefficient),
				"invalid coefficient value", coefficientStr);
		}

		const char* densityStr;
		if (cmd.get("density", densityStr)) {
			errchk(parseInt(densityStr, density),
				"invalid density value", densityStr);
			errchk(density >= 0 && density <= 100,
				"density value out of range", densityStr);
		}

		const char* intervalStr;
		if (cmd.get("interval", intervalStr)) {
			errchk(parseInt(intervalStr, interval),
				"invalid interval value", intervalStr);
			errchk(interval >= 0,
				"negative interval not allowed", intervalStr);
		}

		if (cmd.get("media", media)) {
			errchk(checkFile(media, FileAccessState::R_OK),
				"file inaccessible", media);
		}
	}

	RuntimeConfig::~RuntimeConfig() {

	}
}