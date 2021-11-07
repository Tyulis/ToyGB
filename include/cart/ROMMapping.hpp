#ifndef _CART_ROMMAPPING_HPP
#define _CART_ROMMAPPING_HPP

#include <string>
#include <fstream>

#include "core/OperationMode.hpp"
#include "memory/MemoryMapping.hpp"


namespace toygb {
	class ROMMapping : public MemoryMapping {
		public:
			ROMMapping(uint8_t carttype, std::string romfile, std::string ramfile);
			virtual ~ROMMapping();

			virtual uint8_t get(uint16_t address) = 0;
			virtual void set(uint16_t address, uint8_t value) = 0;

			virtual MemoryMapping* getSRAM() = 0;
			OperationMode getAutoOperationMode();

		protected:
			std::string m_romfile;
			std::string m_sramfile;
			uint8_t* m_romdata;
			uint8_t* m_sramdata;
			uint8_t m_carttype;
	};
}

#endif
