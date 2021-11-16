#include "memory/DMAController.hpp"


namespace toygb {
	DMAController::DMAController() {
		m_oamDmaMapping = nullptr;
	}

	DMAController::~DMAController() {
		if (m_oamDmaMapping != nullptr) delete m_oamDmaMapping;
	}

	void DMAController::init(OperationMode mode){
		m_mode = mode;
		m_oamDmaMapping = new OAMDMAMapping(mode);
	}

	void DMAController::configureMemory(MemoryMap* memory){
		memory->add(IO_OAM_DMA, IO_OAM_DMA, m_oamDmaMapping);
	}

#define dot() co_await std::suspend_always()

	GBComponent DMAController::run(MemoryMap* memory){
		while (true){
			if (m_oamDmaMapping->active){
				if ((m_oamDmaMapping->sourceAddress & 0xFF) >= 0xA0){
					m_oamDmaMapping->active = false;
				} else {
					uint16_t source = m_oamDmaMapping->sourceAddress++;
					uint16_t destination = (source & 0xFF) | 0xFE00;
					memory->set(destination, memory->get(source));
				}
			}
			dot();
		}
	}

	bool DMAController::isOAMDMAActive(){
		return m_oamDmaMapping->active;
	}
}
