#ifndef _CORE_INTERRUPTVECTOR_HPP
#define _CORE_INTERRUPTVECTOR_HPP

#include "core/hardware.hpp"
#include "core/mapping/InterruptRegisterMapping.hpp"
#include "memory/MemoryMap.hpp"
#include "memory/Constants.hpp"
#include "memory/MemoryMap.hpp"
#include "util/enum.hpp"

namespace toygb {
	/** Define all interrupts type, and the default value None (no pending interrupt)
	 *  Integer values are their indices in the registers arrays, and the left shift of their associated bit */
	enum class Interrupt : int {
		None = 0xFF,
		VBlank = 0,
		LCDStat = 1,
		Timer = 2,
		Serial = 3,
		Joypad = 4,
	};

	/** Hold the interrupt status of the Gameboy (request (IF), enable (IE) and master enable (IME)) */
	class InterruptVector {
		public:
			InterruptVector();
			~InterruptVector();

			void init();
			void configureMemory(MemoryMap* memory);

			Interrupt getInterrupt();  // Get the pending interrupt (IE + IF, regardless of IME), or Interrupt::None if there are none
			bool getMaster();          // Get the IME status

			void setEnable(Interrupt interrupt);   // Enable an interrupt (set its bit in the IE register)
			void setRequest(Interrupt interrupt);  // Request an interrupt (set its bit in the IF register)
			void setMaster(bool enable);           // Set the value of IME

			void resetEnable(Interrupt interrupt);   // Disable an interrupt (clear its bit in the IE register)
			void resetRequest(Interrupt interrupt);  // Unset an interrupt request (clear its bit in the IF register)

		private:
			InterruptRegisterMapping* m_enable;   // IE register
			InterruptRegisterMapping* m_request;  // IF register
			bool m_master;  // IME
	};
}

#endif
