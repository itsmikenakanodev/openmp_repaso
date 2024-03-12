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

std::vector<sf::Uint8> image_nueva;
const sf::Uint8 * host_src_image_pixels;

std::tuple<sf::Uint8,sf::Uint8,sf::Uint8> process_pixel_mediana(const sf::Uint8* image, int width, int height, int x, int y) {

        int r = 0;
        int g = 0;
        int b = 0;
        std::vector<sf::Uint8> r_pixels;
        std::vector<sf::Uint8> g_pixels;
        std::vector<sf::Uint8> b_pixels;
        int cc = 0;

        for(int i=x-1;i<=x+1;i++) {
            for(int j=y-1;j<=y+1;j++) {
                int index = (j * width + i)*image_channels;

                if(i>=0 && i<width && j>=0 && j<height) {
                    r_pixels.push_back(image[index]);
                    g_pixels.push_back(image[index + 1]);
                    b_pixels.push_back(image[index + 2]);
                }
            }
        }
        std::sort(r_pixels.begin(), r_pixels.end());
        std::sort(g_pixels.begin(), g_pixels.end());
        std::sort(b_pixels.begin(), b_pixels.end());

        int indiceMediana= r_pixels.size()/2;
        return {r_pixels[indiceMediana], g_pixels[indiceMediana], b_pixels[indiceMediana]};
}

std::tuple<sf::Uint8,sf::Uint8,sf::Uint8> process_pixel_min(const sf::Uint8* image, int width, int height, int x, int y) {

    int r = 0;
    int g = 0;
    int b = 0;
    std::vector<sf::Uint8> r_pixels;
    std::vector<sf::Uint8> g_pixels;
    std::vector<sf::Uint8> b_pixels;
    int cc = 0;

    for(int i=x-1;i<=x+1;i++) {
        for(int j=y-1;j<=y+1;j++) {
            int index = (j * width + i)*image_channels;

            if(i>=0 && i<width && j>=0 && j<height) {
                r_pixels.push_back(image[index]);
                g_pixels.push_back(image[index + 1]);
                b_pixels.push_back(image[index + 2]);
            }
        }
    }
    std::sort(r_pixels.begin(), r_pixels.end());
    std::sort(g_pixels.begin(), g_pixels.end());
    std::sort(b_pixels.begin(), b_pixels.end());

    return {r_pixels[0], g_pixels[0], b_pixels[0]};
}

std::tuple<sf::Uint8,sf::Uint8,sf::Uint8> process_pixel_max(const sf::Uint8* image, int width, int height, int x, int y) {

    int r = 0;
    int g = 0;
    int b = 0;
    std::vector<sf::Uint8> r_pixels;
    std::vector<sf::Uint8> g_pixels;
    std::vector<sf::Uint8> b_pixels;
    int cc = 0;

    for(int i=x-1;i<=x+1;i++) {
        for(int j=y-1;j<=y+1;j++) {
            int index = (j * width + i)*image_channels;

            if(i>=0 && i<width && j>=0 && j<height) {
                r_pixels.push_back(image[index]);
                g_pixels.push_back(image[index + 1]);
                b_pixels.push_back(image[index + 2]);
            }
        }
    }
    std::sort(r_pixels.begin(), r_pixels.end(), std::greater<int>());
    std::sort(g_pixels.begin(), g_pixels.end(),std::greater<int>());
    std::sort(b_pixels.begin(), b_pixels.end(),std::greater<int>());

    return {r_pixels[0], g_pixels[0], b_pixels[0]};
}



void image_median(const sf::Uint8 *image) {
    image_nueva.resize(image_width * image_height * image_channels);

#pragma omp parallel collapsed(2)
    for (int i = 0; i < image_width; i++) {
        for (int j = 0; j < image_height; j++) {

            auto [r, g, b] = process_pixel_mediana(image, image_width, image_height, i,j);

            int index = (j * image_width + i) * image_channels;

            image_nueva[index] = r;
            image_nueva[index + 1] = g;
            image_nueva[index + 2] = b;
            image_nueva[index + 3] = 255;
        }
    }
}

void image_min(const sf::Uint8 *image) {
    image_nueva.resize(image_width * image_height * image_channels);

#pragma omp parallel collapsed(2)
    for (int i = 0; i < image_width; i++) {
        for (int j = 0; j < image_height; j++) {

            auto [r, g, b] = process_pixel_min(image, image_width, image_height, i,j);

            int index = (j * image_width + i) * image_channels;

            image_nueva[index] = r;
            image_nueva[index + 1] = g;
            image_nueva[index + 2] = b;
            image_nueva[index + 3] = 255;
        }
    }
}

void image_max(const sf::Uint8 *image) {
    image_nueva.resize(image_width * image_height * image_channels);

#pragma omp parallel collapsed(2)
    for (int i = 0; i < image_width; i++) {
        for (int j = 0; j < image_height; j++) {

            auto [r, g, b] = process_pixel_max(image, image_width, image_height, i,j);

            int index = (j * image_width + i) * image_channels;

            image_nueva[index] = r;
            image_nueva[index + 1] = g;
            image_nueva[index + 2] = b;
            image_nueva[index + 3] = 255;
        }
    }
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
    image.loadFromFile("D:/Programacion Paralela/openmp_repaso/image02.jpg");
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

    textOptions.setPosition(10, window.getView().getSize().y-40);

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
                    image_median(host_src_image_pixels);
                    auto end = ch::high_resolution_clock::now();
                    ch::duration<double, std::milli> tiempo = end - start;
                    last_time = tiempo.count();
                    //texture.create(image_width, image_height);
                    texture.update(image_nueva.data());
                }
                else if (event.key.scancode == sf::Keyboard::Scan::M) {
                    auto start = ch::high_resolution_clock::now();
                    image_min(host_src_image_pixels);
                    auto end = ch::high_resolution_clock::now();
                    ch::duration<double, std::milli> tiempo = end - start;
                    last_time = tiempo.count();
                    //texture.create(image_width, image_height);
                    texture.update(image_nueva.data());
                }
                else if (event.key.scancode == sf::Keyboard::Scan::N) {
                    auto start = ch::high_resolution_clock::now();
                    image_max(host_src_image_pixels);
                    auto end = ch::high_resolution_clock::now();
                    ch::duration<double, std::milli> tiempo = end - start;
                    last_time = tiempo.count();
                    //texture.create(image_width, image_height);
                    texture.update(image_nueva.data());
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