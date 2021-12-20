#include "memory/mapping/AudioDebugMapping.hpp"

#define OFFSET_START IO_UNDOCUMENTED_FF72
#define OFFSET_FF72  IO_UNDOCUMENTED_FF72 - OFFSET_START
#define OFFSET_FF73  IO_UNDOCUMENTED_FF73 - OFFSET_START
#define OFFSET_FF74  IO_UNDOCUMENTED_FF74 - OFFSET_START
#define OFFSET_FF75  IO_UNDOCUMENTED_FF75 - OFFSET_START
#define OFFSET_PCM12 IO_PCM12 - OFFSET_START
#define OFFSET_PCM34 IO_PCM34 - OFFSET_START


namespace toygb {
	AudioDebugMapping::AudioDebugMapping(HardwareConfig& hardware){
		m_hardware = hardware;

		m_ff72 = 0x00;
		m_ff73 = 0x00;
		m_ff74 = 0x00;
		m_ff75 = 0x00;

		for (int i = 0; i < 4; i++)
			m_amplitudes[i] = 0;
	}

	uint8_t AudioDebugMapping::get(uint16_t address){
		if (m_hardware.isCGBConsole()) {  // FIXME : CGB-only or CGB + AGB, AGB, ... ?
			switch (address){
				case OFFSET_FF72: return m_ff72;
				case OFFSET_FF73: return m_ff73;
				case OFFSET_FF74:
					if (m_hardware.mode() == OperationMode::CGB)
						return m_ff74;
					else
						return 0xFF;
				case OFFSET_FF75: return m_ff75 | 0x8F;
				case OFFSET_PCM12: return m_amplitudes[0] | (m_amplitudes[1] << 4);
				case OFFSET_PCM34: return m_amplitudes[2] | (m_amplitudes[3] << 4);
			}
			std::stringstream errstream;
			errstream << "Wrong memory mapping : " << oh16(address);
			throw EmulationError(errstream.str());
		} else {
			return 0xFF;
		}
	}

	void AudioDebugMapping::set(uint16_t address, uint8_t value){
		if (m_hardware.isCGBConsole()) {
			switch (address) {
				case OFFSET_FF72: m_ff72 = value; break;
				case OFFSET_FF73: m_ff73 = value; break;
				case OFFSET_FF74:
					if (m_hardware.mode() == OperationMode::CGB)
						m_ff74 = value;
					break;
				case OFFSET_FF75: m_ff75 = value & 0x70; break;
				default:
					std::stringstream errstream;
					errstream << "Wrong memory mapping : " << oh16(address);
					throw EmulationError(errstream.str());
					break;
			}
		}
	}

	void AudioDebugMapping::setChannelAmplitude(int channel, uint8_t amplitude){
		m_amplitudes[channel] = amplitude;
	}
}
