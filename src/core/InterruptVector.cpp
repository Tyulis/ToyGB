#include "core/InterruptVector.hpp"


namespace toygb {
	InterruptVector::InterruptVector() {

	}

	void InterruptVector::init(OperationMode mode) {
		m_enable = new InterruptRegisterMapping();
		m_request = new InterruptRegisterMapping();
		m_master = true;
	}

	void InterruptVector::configureMemory(MemoryMap* memory) {
		memory->add(IO_INTERRUPT_ENABLE, IO_INTERRUPT_ENABLE, m_enable);
		memory->add(IO_INTERRUPT_REQUEST, IO_INTERRUPT_REQUEST, m_request);
	}

	Interrupt InterruptVector::getInterrupt(){
		for (int i = 0; i < 5; i++){
			bool enableflag = m_enable->interrupts[i];
			bool requestflag = m_request->interrupts[i];
			if (m_master && enableflag && requestflag){
				return Interrupt(i);
			}
		}
		return Interrupt::None;
	}

	bool InterruptVector::getMaster(){
		return m_master;
	}

	void InterruptVector::setEnable(Interrupt interrupt) {
		if (interrupt != Interrupt::None)
			m_enable->interrupts[enumval(interrupt)] = true;
	}

	void InterruptVector::setRequest(Interrupt interrupt) {
		if (interrupt != Interrupt::None)
			m_request->interrupts[enumval(interrupt)] = true;
	}

	void InterruptVector::resetEnable(Interrupt interrupt) {
		if (interrupt != Interrupt::None)
			m_enable->interrupts[enumval(interrupt)] = false;
	}

	void InterruptVector::resetRequest(Interrupt interrupt) {
		if (interrupt != Interrupt::None)
			m_request->interrupts[enumval(interrupt)] = false;
	}

	void InterruptVector::setMaster(bool enable) {
		m_master = enable;
	}
}
