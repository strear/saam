#include "imageArray.hpp"
#include "media.hpp"
#include "saam.hpp"
#include "tuiapp.hpp"

#include <chrono>
#include <cstdio>
#include <climits>
#include <fstream>
#include <functional>
#include <iterator>
#include <list>
#include <string>

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
		size_t maxWidth = INT_MAX, maxHeight = INT_MAX;
		const char* media = nullptr;
		int coefficient = 603;
		std::list<std::string> files;
		int interval = 6000000;

		RuntimeConfig(Cmdline&, std::function<void(bool, const char*, const char*)>);
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
  --autoscale           dynamically zoom images according to the display size

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

	RuntimeConfig::RuntimeConfig(Cmdline& cmd,
		std::function<void(bool, const char*, const char*)> errchk) {

		monochrome = TextFramebuffer::isMonochrome() || cmd.get("monochrome");
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
			errchk(interval > 0,
				"negative interval not allowed", intervalStr);
		}

		if (cmd.get("media", media)) {
			errchk(accessible(media, FileAccessState::R_OK),
				"file inaccessible", media);
		}

		std::string leftOption = cmd.firstPendOption();
		errchk(leftOption.empty(), "unrecognized parameter", leftOption.c_str());

		cmd.get('-');
		errchk(cmd.remainc() != 0, "missing file operand", nullptr);

		const char* leftParam = cmd.popValue();
		if (leftParam[0] == ':') {
			errchk(accessible(&leftParam[1], FileAccessState::R_OK),
				"file inaccessable", &leftParam[1]);

			std::ifstream is(&leftParam[1]);
			std::string buf{ std::istreambuf_iterator<char>(is),
				std::istreambuf_iterator<char>() };
			const size_t len = buf.length();

			is.close();

			for (size_t entry = 0, next_entry = 0;
				next_entry < len - 1; entry = next_entry + 1) {

				next_entry = buf.find('\n', entry);
				if (entry == next_entry) continue;

				files.push_back(std::string(buf, entry, next_entry - entry));
				errchk(accessible(files.back().c_str(), FileAccessState::R_OK),
					"file inaccessable", files.back().c_str());
			}

			errchk(files.size() > 0, "missing file operand", &leftParam[1]);
		}
		else {
			files.push_back(std::string(leftParam));
			errchk(accessible(files.back().c_str(), FileAccessState::R_OK),
				"file inaccessable", files.back().c_str());

			while (cmd.remainc() != 0) {
				files.push_back(std::string(cmd.popValue()));
				errchk(accessible(files.back().c_str(), FileAccessState::R_OK),
					"file inaccessable", files.back().c_str());
			}

			errchk(files.size() > 0, "missing file operand", nullptr);
		}

		if (intervalStr == nullptr && files.size() > 1) {
			interval = 2000;
		}
	}

	void fit(Array<byte>& dest, size_t width, size_t height) {
		float scale[2] = {};

		if (width * 2.f < dest.sizeOf(1)) {
			scale[1] = dest.sizeOf(1) / 2.f / width;
		}
		if (height * 4.f < dest.sizeOf(0)) {
			scale[0] = dest.sizeOf(0) / 4.f / height;
		}

		if (scale[0] || scale[1]) {
			if (scale[1] > scale[0]) scale[0] = scale[1];

			dest = dest.resample(
				size_t(dest.sizeOf(0) / scale[0]),
				size_t(dest.sizeOf(1) / scale[0]), 4);
		}
	}

	size_t load(Array<byte>* picBuf, RuntimeConfig& conf,
		PicLoader& imread) {

		const size_t fileCount = conf.files.size();
		size_t i = 0;

		while (!conf.files.empty()) {
			imread.loadPic(conf.files.front().c_str());
			imread.getPixels(picBuf[i]);
			conf.files.pop_front();

			fit(picBuf[i], conf.maxWidth, conf.maxHeight);

			printf("\rLoading image %zu / %zu...", ++i, fileCount);
		}

		printf("\n");
		return i;
	}

	void show(Array<byte>* picBuf, size_t picNum, RuntimeConfig& conf,
		Media* bgm, TextFramebuffer& display) {

		auto begintick = std::chrono::high_resolution_clock::now();

		for (size_t i = 0; ; ) {
			if (i >= picNum) {
				if (bgm != nullptr) bgm->pause();

				if (conf.loop) {
					if (bgm != nullptr) bgm->reset();
					i = 0;
					begintick = std::chrono::high_resolution_clock::now();
				}
				else {
					break;
				}
			}
			if (i == 0 && bgm != nullptr) bgm->play();

			display.ready();

			if (conf.autoscale)
				fit(picBuf[i], display.getWidth(), display.getHeight());

			if (conf.monochrome) {
				projectImgMono(display, (ColorRgba*)picBuf[i].cptr(),
					picBuf[i].sizeOf(1), picBuf[i].sizeOf(0),
					conf.reverse, conf.density, conf.coefficient);
			}
			else {
				projectImg(display, (ColorRgba*)picBuf[i].cptr(),
					picBuf[i].sizeOf(1), picBuf[i].sizeOf(0),
					!conf.grayscale, conf.shading, conf.reverse, conf.density,
					conf.shadingThreshold, conf.coefficient);
			}

			display.flush(conf.monochrome && !conf.reverse, conf.noskip);
			
			const clock_t tick =
				std::chrono::duration_cast<std::chrono::duration<clock_t, std::milli>>(
					std::chrono::high_resolution_clock::now() - begintick).count();

			i = tick / conf.interval + 1;
			sleep(conf.interval * i - tick);
		}
	}
}

int main(int argc, char** argv) {
	Cmdline cmd(argc, argv);

	if (cmd.get("help")) {
		help(cmd.appname());
		return 0;
	}

	auto errchk = [&](bool flag,
		const char* errtype, const char* detail = nullptr) {
			if (flag) return;

			fprintf(stderr, "%s: %s", cmd.appname(), errtype);
			if (detail != nullptr) fprintf(stderr, " -- '%s'", detail);
			fprintf(stderr, "\nTry '--help' for more information.\n");

			exit(errtype[0]);
	};

	RuntimeConfig conf(cmd, errchk);

	PicLoader imread;

	Array<byte>* picBuf = new Array<byte>[conf.files.size()];
	errchk(picBuf != nullptr, "out of memory");
	size_t picNum = load(picBuf, conf, imread);

	TextFramebuffer display;

	Media* bgm = nullptr;

	if (conf.media != nullptr) bgm = new Media(conf.media);

	show(picBuf, picNum, conf, bgm, display);

	if (bgm != nullptr) delete bgm;
	delete[] picBuf;

	return 0;
}