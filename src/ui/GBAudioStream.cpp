#include "ui/GBAudioStream.hpp"

namespace toygb {
	GBAudioStream::GBAudioStream() : sf::SoundStream() {

	}

	void GBAudioStream::init(AudioController* controller, int channel){
		m_controller = controller;
		m_channel = channel;
		initialize(2, OUTPUT_SAMPLE_FREQUENCY);
	}

	bool GBAudioStream::onGetData(sf::SoundStream::Chunk& data){
		//std::cout << "Read " << numSamples << " samples";
		int16_t* sampleBuffer;
		while ((sampleBuffer = m_controller->getSamples(m_channel)) == nullptr);
		/*for (int i = 0; i < OUTPUT_BUFFER_SAMPLES; i++){
			std::cout << m_sampleBuffer[i] << " ";
		}
		std::cout << " //" << std::endl;*/

		data.samples = sampleBuffer;
		data.sampleCount = OUTPUT_BUFFER_SAMPLES*2;
		//std::cout << ", return " << data.sampleCount << " samples, rate " << getSampleRate() << " on " << getChannelCount() << " channels" << std::endl;
		return true;
	}

	void GBAudioStream::onSeek(sf::Time offset){

	}
}
