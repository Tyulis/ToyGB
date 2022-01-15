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
		m_memory = memory;

		while (true) {
			// OAM DMA
			// FIXME : There are apparently many obscure shenanigans with OAM DMA, check them out

			// A DMA routine is running and actively copying data
			// A previous DMA can still be running while a new one is in its startup phase
			if (m_oamDmaMapping->active) {
				// Transferring a byte per machine cycle
				uint16_t source = m_oamDmaMapping->sourceAddress++;
				uint16_t destination = (source & 0xFF) | 0xFE00;
				memory->set(destination, memory->get(source));

				// Source address got over 0x--9F -> transfer to OAM finished
				if ((m_oamDmaMapping->sourceAddress & 0xFF) >= 0xA0)
					m_oamDmaMapping->active = false;
			}

			// A DMA routine has been requested by writing to FF46 but still in the startup cycle
			if (m_oamDmaMapping->requested) {
				m_oamDmaMapping->idleCycles -= 1;
				if (m_oamDmaMapping->idleCycles < 0) {
					m_oamDmaMapping->requested = false;
					m_oamDmaMapping->active = true;
					m_oamDmaMapping->sourceAddress = (m_oamDmaMapping->requestedAddress >= 0xE000 ? m_oamDmaMapping->requestedAddress - 0x2000 : m_oamDmaMapping->requestedAddress);
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
	bool DMAController::isOAMDMAActive() const {
		return m_oamDmaMapping->active;
	}

	// Tell whether a memory operation at the given address can conflict with OAM DMA (same bus)
	bool DMAController::isConflicting(uint16_t address) const {
		// No OAM DMA = no conflict
		if (!m_oamDmaMapping->active)
			return false;
		// External bus conflict
		else if (((/*EXTERNAL_BUS_LOW_START <= address && */address <= EXTERNAL_BUS_LOW_END) || (EXTERNAL_BUS_HIGH_START <= address && address <= EXTERNAL_BUS_HIGH_END)) &&
				((/*EXTERNAL_BUS_LOW_START <= m_oamDmaMapping->sourceAddress && */m_oamDmaMapping->sourceAddress <= EXTERNAL_BUS_LOW_END) || (EXTERNAL_BUS_HIGH_START <= m_oamDmaMapping->sourceAddress && m_oamDmaMapping->sourceAddress <= EXTERNAL_BUS_HIGH_END)))
			return true;
		// Video RAM bus conflict
		else if ((VIDEO_BUS_START <= address && address <= VIDEO_BUS_END) && (VIDEO_BUS_START <= m_oamDmaMapping->sourceAddress && m_oamDmaMapping->sourceAddress <= VIDEO_BUS_END))
			return true;
		// No conflict
		else
			return false;
	}

	// Get the value that the CPU will read at the given address, accounting for bus conflicts
	uint8_t DMAController::conflictingRead(uint16_t address) {
		// In case of read/read bus conflict, OAM DMA wins
		if (isConflicting(address))
			return m_memory->get(m_oamDmaMapping->sourceAddress);
		else
			return m_memory->get(address);
	}
}
