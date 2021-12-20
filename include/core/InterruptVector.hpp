#ifndef _CORE_INTERRUPTVECTOR_HPP
#define _CORE_INTERRUPTVECTOR_HPP

#include "core/hardware.hpp"
#include "memory/MemoryMap.hpp"
#include "memory/Constants.hpp"
#include "memory/MemoryMap.hpp"
#include "memory/mapping/InterruptRegisterMapping.hpp"
#include "util/bitwise.hpp"
#include "util/enum.hpp"

namespace toygb {
	enum class Interrupt : int {
		None = 0xFF,
		VBlank = 0,
		LCDStat = 1,
		Timer = 2,
		Serial = 3,
		Joypad = 4,
	};

	class InterruptVector {
		public:
			InterruptVector();

			void init(HardwareConfig& mode);
			void configureMemory(MemoryMap* memory);

			Interrupt getInterrupt();
			bool getMaster();

			void setEnable(Interrupt interrupt);
			void setRequest(Interrupt interrupt);
			void setMaster(bool enable);

			void resetEnable(Interrupt interrupt);
			void resetRequest(Interrupt interrupt);

		private:
			InterruptRegisterMapping* m_enable;
			InterruptRegisterMapping* m_request;
			bool m_master;
	};
}

#endif
