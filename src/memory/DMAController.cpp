#include "memory/DMAController.hpp"


namespace toygb {
	// Initialize the component with null values (actual initialization is in DMAController::init)
	DMAController::DMAController() {
		m_oamDmaMapping = nullptr;
	}

	DMAController::~DMAController() {
		if (m_oamDmaMapping != nullptr) delete m_oamDmaMapping;
		m_oamDmaMapping = nullptr;
	}

	// Initialize the component
	void DMAController::init(HardwareStatus* hardware) {
		m_hardware = hardware;
		m_oamDmaMapping = new OAMDMAMapping(hardware);
		m_cyclesToSkip = 0;
	}

	// Configure the associated memory mappings
	void DMAController::configureMemory(MemoryMap* memory) {
		memory->add(IO_OAM_DMA, IO_OAM_DMA, m_oamDmaMapping);
	}

// Make the coroutine wait till the next clock
#define clock(num) m_cyclesToSkip = num - 1;  \
					co_await std::suspend_always()

	// Main coroutine component
	GBComponent DMAController::run(MemoryMap* memory) {
		while (true) {
			// OAM DMA
			// FIXME : There are apparently many obscure shenanigans with OAM DMA, check them out

			// A DMA routine is running and actively copying data
			// A previous DMA can still be running while a new one is in its startup phase
			if (m_oamDmaMapping->active) {
				// Transferring a byte per clock tick
				uint16_t source = m_oamDmaMapping->sourceAddress++;
				uint16_t destination = (source & 0xFF) | 0xFE00;
				memory->set(destination, memory->get(source));

				// Source address got over 0x--9F -> transfer to OAM finished
				if ((m_oamDmaMapping->sourceAddress & 0xFF) >= 0xA0)
					m_oamDmaMapping->active = false;
			}

			// A DMA routine has been requested by writing to FF46 but still in the 4 startup clocks
			if (m_oamDmaMapping->requested) {
				m_oamDmaMapping->idleCycles -= 1;
				if (m_oamDmaMapping->idleCycles < 0) {
					m_oamDmaMapping->requested = false;
					m_oamDmaMapping->active = true;
					m_oamDmaMapping->sourceAddress = m_oamDmaMapping->requestedAddress;
				}
			}

			clock(4);
		}
	}

	// Tell whether the emulator can skip running this component for the cycle, to save a context commutation if running it is useless
	bool DMAController::skip() {
		if (m_cyclesToSkip > 0) {
			m_cyclesToSkip -= 1;
			return true;
		}
		// Skip if no DMA operation is active. TODO : HDMA
		return !(m_oamDmaMapping->active || m_oamDmaMapping->requested);
	}

	// Tell whether a OAM DMA operation is active
	bool DMAController::isOAMDMAActive() {
		return m_oamDmaMapping->active;
	}
}
