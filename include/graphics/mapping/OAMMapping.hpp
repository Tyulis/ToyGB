#ifndef _GRAPHICS_MAPPING_OAMMAPPING_HPP
#define _GRAPHICS_MAPPING_OAMMAPPING_HPP

#include "core/hardware.hpp"
#include "graphics/mapping/LCDMemoryMapping.hpp"
#include "memory/Constants.hpp"

namespace toygb {
	/** OAM area memory mapping. Also manages the "forbidden" area at 0xFEA0-0xFEFF,
	 *  that is unrelated to OAM but it is contiguous and causes OAM corruption in the same way */
	class OAMMapping : public LCDMemoryMapping {
		public:
			OAMMapping(HardwareStatus* hardware, uint8_t* array);
			~OAMMapping();

			// CPU access, unavailable while the PPU is accessing it
			virtual uint8_t get(uint16_t address);
			virtual void set(uint16_t address, uint8_t value);

			// PPU access, unavailable when the PPU has not reserved it
			virtual uint8_t lcdGet(uint16_t address);
			virtual void lcdSet(uint16_t address, uint8_t value);

		protected:
			HardwareStatus* m_hardware;

			// 0xFEA0-0xFEFF area emulation
			uint8_t* m_fea0;
			uint8_t* m_fec0;
			uint8_t* m_fee0;
	};
}

#endif
