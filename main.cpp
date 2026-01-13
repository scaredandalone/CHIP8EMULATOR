#include <iostream>
#include <chip8.h>
#include <SFML/Graphics.hpp>
#include <SFML/Window.hpp>
#include <cstdlib>

int main() {
    Chip8 chip8;
    chip8.initialize();
    chip8.loadROM("C:/Users/cosmi/source/repos/CHIP8EMULATOR/roms/Maze.ch8");

    sf::RenderWindow window(sf::VideoMode({ 640, 320 }), "Chip-8 Emulator");

    while (window.isOpen()) {
        while (const std::optional event = window.pollEvent()) {
            if (event->is<sf::Event::Closed>())
                window.close();
        }

        // Run emulation cycle
        chip8.cycle();

        // Draw if needed
        if (chip8.shouldDraw()) {
            window.clear(sf::Color::Black);

            // Draw each pixel
            for (int x = 0; x < 64; x++) {
                for (int y = 0; y < 32; y++) {
                    if (chip8.getPixel(x, y) != 0) {
                        sf::RectangleShape pixel(sf::Vector2f(10, 10));
                        pixel.setPosition(sf::Vector2f(x * 10 , y * 10));
                        pixel.setFillColor(sf::Color::White);
                        window.draw(pixel);
                    }
                }
            }

            window.display();
            chip8.clearDrawFlag();
        }
    }

    return 0;
}