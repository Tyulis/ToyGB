#ifndef _MEMORY_MAPPING_BANKEDWRAMMAPPING_HPP
#define _MEMORY_MAPPING_BANKEDWRAMMAPPING_HPP

#include "memory/MemoryMapping.hpp"


namespace toygb {
	/** Base class for array memory mappings with switchable banks */
	class BankedMemoryMapping : public MemoryMapping {
		public:
			/** Initialize the memory mapping. bankSelect is a register that defines the currently selected bank, numBanks is the amount of available banks
			 *  bankSize is the size of a single bank, array is the actual mapped array (of size numBanks*bankSize),
			 *  accessible tells whether the mapping is accessible to the CPU from startup */
			BankedMemoryMapping(uint8_t* bankSelect, int numBanks, uint16_t bankSize, uint8_t* array, bool accessible);

			virtual uint8_t get(uint16_t address) = 0;
			virtual void set(uint16_t address, uint8_t value) = 0;

			virtual void load(std::istream& input);
			virtual void save(std::ostream& output);

			bool accessible;

		protected:
			int m_numBanks;
			uint8_t* m_bankSelect;
			uint16_t m_bankSize;
			uint8_t* m_array;
	};
}

#endif
