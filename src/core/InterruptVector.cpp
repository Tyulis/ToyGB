#include "core/InterruptVector.hpp"


namespace toygb {
	// Initialize the interrupt vector with null values (actual initialization is in InterruptVector::init)
	InterruptVector::InterruptVector() {
		m_enable = nullptr;
		m_request = nullptr;
		m_master = true;
	}

	InterruptVector::~InterruptVector() {
		if (m_enable != nullptr) delete m_enable;
		if (m_request != nullptr) delete m_request;
		m_enable = m_request = nullptr;
	}

	// Initialize the interrupt vector
	void InterruptVector::init() {
		m_enable = new InterruptRegisterMapping(true);    // IE can keep a value in its upper unused bits
		m_request = new InterruptRegisterMapping(false);  // IF can't
		m_master = true;
	}

	// Configure the associated memory mappings
	void InterruptVector::configureMemory(MemoryMap* memory) {
		memory->add(IO_INTERRUPT_ENABLE, IO_INTERRUPT_ENABLE, m_enable);
		memory->add(IO_INTERRUPT_REQUEST, IO_INTERRUPT_REQUEST, m_request);
	}


	// Get the pending interrupt (IE + IF, regardless of IME), or Interrupt::None
	Interrupt InterruptVector::getInterrupt(){
		for (int i = 0; i < 5; i++){
			bool enableflag = m_enable->interrupts[i];
			bool requestflag = m_request->interrupts[i];
			if (enableflag && requestflag){
				return Interrupt(i);
			}
		}
		return Interrupt::None;
	}

	// Get the IME status
	bool InterruptVector::getMaster(){
		return m_master;
	}

	// Set the interrupt's bit in IE
	void InterruptVector::setEnable(Interrupt interrupt) {
		if (interrupt != Interrupt::None)
			m_enable->interrupts[enumval(interrupt)] = true;
	}

	// Request an interrup : set the interrupt's bit in IF
	void InterruptVector::setRequest(Interrupt interrupt) {
		if (interrupt != Interrupt::None)
			m_request->interrupts[enumval(interrupt)] = true;
	}

	// Reset the interrupt's bit in IE
	void InterruptVector::resetEnable(Interrupt interrupt) {
		if (interrupt != Interrupt::None)
			m_enable->interrupts[enumval(interrupt)] = false;
	}

	// Reset the interrupt's request bit in IF
	void InterruptVector::resetRequest(Interrupt interrupt) {
		if (interrupt != Interrupt::None)
			m_request->interrupts[enumval(interrupt)] = false;
	}

	// Set the value of IME
	void InterruptVector::setMaster(bool enable) {
		m_master = enable;
	}
}
