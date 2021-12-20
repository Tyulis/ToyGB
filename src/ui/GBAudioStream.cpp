#include "ui/GBAudioStream.hpp"

namespace toygb {
	GBAudioStream::GBAudioStream() : sf::SoundStream() {
		m_sampleBuffer = new int16_t[2*OUTPUT_BUFFER_SAMPLES];
	}

	GBAudioStream::~GBAudioStream() {
		if (m_sampleBuffer != nullptr) delete[] m_sampleBuffer;
		m_sampleBuffer = nullptr;
	}

	void GBAudioStream::init(AudioController* controller){
		m_controller = controller;
		initialize(2, OUTPUT_SAMPLE_FREQUENCY);
	}

	bool GBAudioStream::onGetData(sf::SoundStream::Chunk& data){
		while (!m_controller->getSamples(m_sampleBuffer));

		data.samples = m_sampleBuffer;
		data.sampleCount = OUTPUT_BUFFER_SAMPLES*2;

		return true;
	}

	void GBAudioStream::onSeek(sf::Time offset){

	}
}
