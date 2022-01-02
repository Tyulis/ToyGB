#ifndef _CORE_MAPPING_BOOTROMDISABLEMAPPING_HPP
#define _CORE_MAPPING_BOOTROMDISABLEMAPPING_HPP

#include "core/hardware.hpp"
#include "memory/MemoryMapping.hpp"

namespace toygb {
	/** BootROM disable IO register memory mapping */
	class BootromDisableMapping : public MemoryMapping {
		public:
			BootromDisableMapping(HardwareStatus* hardware);

			uint8_t get(uint16_t address);
			void set(uint16_t address, uint8_t value);

		protected:
			HardwareStatus* m_hardware;
	};
}

#endif
