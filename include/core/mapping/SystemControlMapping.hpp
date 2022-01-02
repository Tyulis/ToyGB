#ifndef _CORE_MAPPING_SYSTEMCONTROLMAPPING_HPP
#define _CORE_MAPPING_SYSTEMCONTROLMAPPING_HPP

#include "core/hardware.hpp"
#include "memory/Constants.hpp"
#include "memory/MemoryMapping.hpp"

namespace toygb {
	/** System control KEY registers memory mapping */
	class SystemControlMapping : public MemoryMapping {
		public:
			SystemControlMapping(HardwareStatus* hardware);

			uint8_t get(uint16_t address);
			void set(uint16_t address, uint8_t value);

			bool prepareSpeedSwitch() const;

		protected:
			HardwareStatus* m_hardware;
			bool m_prepareSpeedSwitch;
	};
}

#endif
