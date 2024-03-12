#include <iostream>
#include <iostream>
#include <vector>
#include <omp.h>
#include <SFML/Graphics.hpp>
#include <fmt/core.h>
#include <chrono>

namespace ch = std::chrono;

static int image_width;
static int image_height;
static int image_channels = 4;
double last_time = 0;
static bool first = false;

std::vector<sf::Uint8> image_rotated;
const sf::Uint8 * host_src_image_pixels;

void rotate_image(const sf::Uint8* image) {
    image_rotated.resize(image_width*image_height*image_channels);

#pragma omp parallel for collapse(2) default(none) shared(image_height,image_width,image_rotated,image)
    for (int y = 0; y < image_height; ++y) {
        for (int x = 0; x < image_width; ++x) {
            int index = (y * image_width + x) * 4; //(0*5+0)*4 = 0
            int rotatedIndex = ((image_width - x - 1) * image_height + y) * 4; // ((5-0-1) *5 +0) *4=80

            image_rotated[rotatedIndex] = image[index];
            image_rotated[rotatedIndex + 1] = image[index + 1];
            image_rotated[rotatedIndex + 2] = image[index + 2];
            image_rotated[rotatedIndex + 3] = image[index + 3];
        }
    }
/*#pragma omp parallel for
    for (int i = 0; i < image_height; i++) {
        int start =(i*image_width*4);
        int end = start+(image_width*4)-1;
        int condicion = end;
        while (start < condicion) {
            image_rotated[start]=image[end-3];
            image_rotated[start+1]=image[end-2];
            image_rotated[start+2]=image[end-1];
            image_rotated[start+3]=image[end];
            start+=4;
            end-=4;
        }
    }*/

}
int main(int argc, char **argv) {


    sf::Text text;
    sf::Font font;
    {
        font.loadFromFile("D:/Programacion Paralela/openmp_repaso/arial.ttf");
        text.setFont(font);
        text.setString("Mandelbrot set");
        text.setCharacterSize(24); // in pixels, not points!
        text.setFillColor(sf::Color::White);
        text.setStyle(sf::Text::Bold);
        text.setPosition(10, 10);
    }

    sf::Text textOptions;
    {
        font.loadFromFile("D:/Programacion Paralela/openmp_repaso/arial.ttf");
        textOptions.setFont(font);
        textOptions.setCharacterSize(24);
        textOptions.setFillColor(sf::Color::White);
        textOptions.setStyle(sf::Text::Bold);
        textOptions.setString("OPTIONS: [R] Reset [B] Blur");
    }

    //load image
    sf::Image image;
    image.loadFromFile("D:/Programacion Paralela/openmp_repaso/image01.jpg");
    host_src_image_pixels = image.getPixelsPtr();

    sf::Image rotatedImage;

    image_width = image.getSize().x;
    image_height = image.getSize().y;
    image_channels = 4;

    sf::Texture texture;
    texture.create(image_width, image_height);
    texture.update(image.getPixelsPtr());

    int w = 1600;
    int h = 900;

    sf::RenderWindow window(sf::VideoMode(w, h), "OMP Blur example");

    sf::Sprite sprite;
    {
        sprite.setTexture(texture);

        float scaleFactorX = w * 1.0 / image.getSize().x;
        float scaleFactorY = h * 1.0 / image.getSize().y;
        sprite.scale(scaleFactorX, scaleFactorY);
    }

    sf::Clock clock;

    sf::Clock clockFrames;
    int frames = 0;
    int fps = 0;

    //textOptions.setPosition(10, window.getView().getSize().y-40);

    while (window.isOpen()) {
        sf::Event event;
        while (window.pollEvent(event)) {
            if (event.type == sf::Event::Closed) {
                window.close();
            } else if (event.type == sf::Event::KeyReleased) {
                if (event.key.scancode == sf::Keyboard::Scan::R) {
                    texture.update(image.getPixelsPtr());
                    last_time = 0;
                } else if (event.key.scancode == sf::Keyboard::Scan::B) {
                    auto start = ch::high_resolution_clock::now();
                    if(!first){
                        rotate_image(host_src_image_pixels);
                        first=true;
                    }else{
                        rotate_image(rotatedImage.getPixelsPtr());
                    }
                    auto end = ch::high_resolution_clock::now();
                    auto tmp = image_width;
                    image_width = image_height;
                    image_height = tmp;
                    rotatedImage.create(image_width, image_height, image_rotated.data());

                    ch::duration<double, std::milli> tiempo = end - start;
                    last_time = tiempo.count();
                    //texture.create(image_width, image_height);
                    texture.update(rotatedImage.getPixelsPtr());
                }
            } else if (event.type == sf::Event::Resized) {
                float scaleFactorX = event.size.width * 1.0 / image.getSize().x;
                float scaleFactorY = event.size.height * 1.0 / image.getSize().y;

                sprite = sf::Sprite();
                sprite.setTexture(texture);
                sprite.scale(scaleFactorX, scaleFactorY);
            }
        }

        if (clockFrames.getElapsedTime().asSeconds() >= 1.0) {
            fps = frames;
            frames = 0;
            clockFrames.restart();
        }
        frames++;

        window.clear(sf::Color::Black);
        window.draw(sprite);
        window.draw(text);
        window.draw(textOptions);
        window.display();
    }


    return 0;
}

