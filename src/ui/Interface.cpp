#include "ui/Interface.hpp"


namespace toygb {
	Interface::Interface(){

	}

	void Interface::run(LCDController* lcd, JoypadController* joypad){
		m_lcd = lcd;
		m_joypad = joypad;

		sf::RenderWindow window(sf::VideoMode(LCD_WIDTH, LCD_HEIGHT), "ToyGB");
		sf::Texture display;
		if (!display.create(LCD_WIDTH, LCD_HEIGHT))
			window.close();

		sf::Uint8* pixels = new sf::Uint8[LCD_WIDTH * LCD_HEIGHT * 4];
		sf::Sprite screen;
		screen.setTexture(display);
		screen.setPosition(sf::Vector2f(0.0f, 0.0f));

		while (window.isOpen()) {
			sf::Event event;
			while (window.pollEvent(event)) {
				if (event.type == sf::Event::Closed)
					window.close();
			}

			uint16_t* gbValues = m_lcd->pixels();
			for (int x = 0; x < LCD_WIDTH; x++){
				for (int y = 0; y < LCD_HEIGHT; y++){
					uint16_t gbValue = gbValues[y*LCD_WIDTH + x];
					pixels[4*(y*LCD_WIDTH + x)] = ((gbValue & 0x1F) << 3) + 0b101;
					pixels[4*(y*LCD_WIDTH + x) + 1] = (((gbValue >> 5) & 0x1F) << 3) + 0b101;
					pixels[4*(y*LCD_WIDTH + x) + 2] = (((gbValue >> 10) & 0x1F) << 3) + 0b101;
					pixels[4*(y*LCD_WIDTH + x) + 3] = 255;
				}
			}

			display.update(pixels);
			screen.setTexture(display);
			window.clear();
			window.draw(screen);
			window.display();
		}
	}
}
