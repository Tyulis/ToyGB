#ifndef _UI_GBAUDIOSTREAM_HPP
#define _UI_GBAUDIOSTREAM_HPP

#include <cstdint>
#include <SFML/Audio.hpp>

#include "audio/timing.hpp"
#include "audio/AudioController.hpp"


namespace toygb {
	class GBAudioStream : public sf::SoundStream {
		public:
			GBAudioStream();

			void init(AudioController* controller);

		protected:
			bool onGetData(sf::SoundStream::Chunk& data);
			void onSeek(sf::Time timeOffset);

			AudioController* m_controller;
			int16_t* m_sampleBuffer;
			bool m_started;
	};
}

#endif
