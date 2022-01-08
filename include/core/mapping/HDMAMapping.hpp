#ifndef _MEMORY_MAPPING_HDMAMAPPING_HPP
#define _MEMORY_MAPPING_HDMAMAPPING_HPP

#include "memory/Constants.hpp"
#include "memory/MemoryMapping.hpp"
#include "util/error.hpp"


namespace toygb {
	/** HDMA IO registers memory mapping
	 *  TODO : This is currently a stub */
	class HDMAMapping : public MemoryMapping {
		public:
			HDMAMapping();

			virtual uint8_t get(uint16_t address);
			virtual void set(uint16_t address, uint8_t value);

			bool running() const;
			void nextBlock();

			uint16_t source;  // DMA source address (registers HDMA1, HDMA2)
			uint16_t dest;    // DMA destination address (registers HDMA3, HDMA4)
			bool type;        // Transfer type (0 = general-purpose, 1 = HBlank) (register HDMA5, bit 7)
			uint16_t blocks;  // Number of 16-bytes blocks to transfer (register HDMA5, bits 0-6 +1)
			bool active;      // Whether a HDMA operation is active
			bool paused;      // Whether a HDMA operation is active but paused
	};
}

#endif
