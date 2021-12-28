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
	void DMAController::init(HardwareConfig& hardware) {
		m_hardware = hardware;
		m_oamDmaMapping = new OAMDMAMapping(hardware);
	}

	// Configure the associated memory mappings
	void DMAController::configureMemory(MemoryMap* memory) {
		memory->add(IO_OAM_DMA, IO_OAM_DMA, m_oamDmaMapping);
	}

// Make the coroutine wait till the next clock
#define dot() co_await std::suspend_always()

	// Main coroutine component
	GBComponent DMAController::run(MemoryMap* memory) {
		while (true) {
			// OAM DMA
			// FIXME : There are apparently many obscure shenanigans with OAM DMA, check them out
			if (m_oamDmaMapping->active) {
				// Source address got over 0x9F -> transfer to OAM finished
				if ((m_oamDmaMapping->sourceAddress & 0xFF) >= 0xA0) {
					m_oamDmaMapping->active = false;
				} else {  // Transferring a byte per clock tick
					uint16_t source = m_oamDmaMapping->sourceAddress++;
					uint16_t destination = (source & 0xFF) | 0xFE00;
					memory->set(destination, memory->get(source));
				}
			}
			dot();
		}
	}

	// Tell whether a OAM DMA operation is active
	bool DMAController::isOAMDMAActive() {
		return m_oamDmaMapping->active;
	}
}
