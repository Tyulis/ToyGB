#include "ui/Interface.hpp"


namespace toygb {
	Interface::Interface(){

	}

	void Interface::operator()(LCDController* lcd, JoypadController* joypad){
		m_lcd = lcd;
		m_joypad = joypad;
	}
}
