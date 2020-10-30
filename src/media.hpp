#pragma once

namespace Saam {
	class Media {
		void* impl = nullptr;
		Media(const Media&) = delete;

	public:
		Media(const char* file);
		~Media();

		void play();
		void pause();
		void reset();
	};
}