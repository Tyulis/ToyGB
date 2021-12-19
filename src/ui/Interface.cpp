#include "ui/Interface.hpp"

#define PIXEL_SIZE 4
#define WINDOW_WIDTH LCD_WIDTH*PIXEL_SIZE
#define WINDOW_HEIGHT LCD_HEIGHT*PIXEL_SIZE

namespace toygb {
	Interface::Interface(){
		m_lcd = nullptr;
		m_audio = nullptr;
		m_joypad = nullptr;
		m_stopping = false;
	}


	void Interface::run(LCDController* lcd, AudioController* audio, JoypadController* joypad){
		m_lcd = lcd;
		m_audio = audio;
		m_joypad = joypad;

		sf::RenderWindow window(sf::VideoMode(WINDOW_WIDTH, WINDOW_HEIGHT), "ToyGB");
		sf::Texture display;
		if (!display.create(WINDOW_WIDTH, WINDOW_HEIGHT))
			window.close();

		sf::Uint8* pixels = new sf::Uint8[WINDOW_WIDTH * WINDOW_HEIGHT * 4];
		sf::Sprite screen;
		screen.setTexture(display);
		screen.setPosition(sf::Vector2f(0.0f, 0.0f));

		setupAudio();

		window.setFramerateLimit(60);
		while (window.isOpen()) {
			sf::Event event;
			while (window.pollEvent(event)) {
				if (event.type == sf::Event::Closed){
					window.close();
					m_audioStream.stop();
					m_stopping = true;
				}
			}

			updateJoypad();
			updateGraphics(pixels);

			display.update(pixels);
			screen.setTexture(display);
			window.clear();
			window.draw(screen);
			window.display();
		}
	}

	void Interface::updateJoypad(){
		m_joypad->setButton(JoypadButton::Up, sf::Keyboard::isKeyPressed(sf::Keyboard::Up));
		m_joypad->setButton(JoypadButton::Down, sf::Keyboard::isKeyPressed(sf::Keyboard::Down));
		m_joypad->setButton(JoypadButton::Left, sf::Keyboard::isKeyPressed(sf::Keyboard::Left));
		m_joypad->setButton(JoypadButton::Right, sf::Keyboard::isKeyPressed(sf::Keyboard::Right));
		m_joypad->setButton(JoypadButton::A, sf::Keyboard::isKeyPressed(sf::Keyboard::K));
		m_joypad->setButton(JoypadButton::B, sf::Keyboard::isKeyPressed(sf::Keyboard::L));
		m_joypad->setButton(JoypadButton::Start, sf::Keyboard::isKeyPressed(sf::Keyboard::O));
		m_joypad->setButton(JoypadButton::Select, sf::Keyboard::isKeyPressed(sf::Keyboard::M));
	}

	void Interface::updateGraphics(sf::Uint8* pixels){
		uint16_t* gbValues = m_lcd->pixels();
		for (int x = 0; x < LCD_WIDTH; x++){
			for (int y = 0; y < LCD_HEIGHT; y++){
				uint16_t gbValue = gbValues[y*LCD_WIDTH + x];
				uint8_t red = ((gbValue & 0x1F) << 3) + 0b101;
				uint8_t green = (((gbValue >> 5) & 0x1F) << 3) + 0b101;
				uint8_t blue = (((gbValue >> 10) & 0x1F) << 3) + 0b101;
				uint8_t alpha = 255;
				for (int xpix = 0; xpix < PIXEL_SIZE; xpix++){
					for (int ypix = 0; ypix < PIXEL_SIZE; ypix++){
						int index = 4*((y*PIXEL_SIZE + ypix)*WINDOW_WIDTH + (x*PIXEL_SIZE + xpix));
						pixels[index] = red;
						pixels[index + 1] = green;
						pixels[index + 2] = blue;
						pixels[index + 3] = alpha;
					}
				}
			}
		}
	}

	void Interface::setupAudio(){
		m_audioStream.init(m_audio);
		m_audioStream.play();
	}

	bool Interface::isStopping(){
		return m_stopping;
	}
}
