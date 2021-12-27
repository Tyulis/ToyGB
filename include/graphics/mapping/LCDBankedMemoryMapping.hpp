#ifndef _GRAPHICS_MAPPING_LCDBANKEDMEMORYMAPPING_HPP
#define _GRAPHICS_MAPPING_LCDBANKEDMEMORYMAPPING_HPP

#include "graphics/mapping/LCDMemoryMapping.hpp"


namespace toygb {
	/** Banked memory memory with PPU access */
	class LCDBankedMemoryMapping : public LCDMemoryMapping {
		public:
			LCDBankedMemoryMapping(uint8_t* bankSelect, uint16_t bankSize, uint8_t* array);

			// Access from the CPU (blocked when accessed by the PPU)
			virtual uint8_t get(uint16_t address);
			virtual void set(uint16_t address, uint8_t value);

			// Access from the PPU (blocked when not reserved by the PPU)
			virtual uint8_t lcdGet(uint16_t address);
			virtual void lcdSet(uint16_t address, uint8_t value);

		private:
			uint8_t* m_bankSelect;
			uint16_t m_bankSize;
	};
}

#endif
