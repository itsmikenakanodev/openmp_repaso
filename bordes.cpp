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

std::tuple<sf::Uint8, sf::Uint8, sf::Uint8> process_pixel_edge(const sf::Uint8 *image, int width, int height, int pos) {
    int r = 0;
    int g = 0;
    int b = 0;

    // Realce
    //int matriz[] = { 0, 0, 0, -1, 1, 0, 0, 0, 0 };

    // Detectar
    int matriz[] = { 0, 1, 0, 1, -4, 1, 0, 1, 0 };
    //repujado
    //int matriz[] = { -2,-1,0,-1,1,1,0,1,2 };

    for (int i = -1; i <= 1; i++) {
        for (int j = -1; j <= 1; j++) {
            int index = (pos * 4) + (i * 4) + (j * width * 4);

            if (index >= 0 && index <= width * height * 4) {
                int matrixIndex = (i + 1) * 3 + (j + 1);
                int weight = matriz[matrixIndex];

                r += image[index] * weight;
                g += image[index + 1] * weight;
                b += image[index + 2] * weight;
            }
        }
    }

    r = std::clamp(r, 0, 255);
    g = std::clamp(g, 0, 255);
    b = std::clamp(b, 0, 255);

    return { r, g, b };
}

void blur_image_edge(const sf::Uint8 *image) {
    image_nueva.resize(image_width * image_height * image_channels);

#pragma omp parallel collapsed(2)
    for (int i = 0; i < image_width; i++) {
        for (int j = 0; j < image_height; j++) {

            auto [r, g, b] = process_pixel_edge(image, image_width, image_height, j * image_width + i);

            int index = (j * image_width + i) * image_channels;

            image_nueva[index] = r;
            image_nueva[index + 1] = g;
            image_nueva[index + 2] = b;
            image_nueva[index + 3] = 255;
        }
    }
}

std::tuple<sf::Uint8, sf::Uint8, sf::Uint8>
process_pixel_sobel(const sf::Uint8 *image, int width, int height, int pos) {
    int r_x = 0;
    int g_x = 0;
    int b_x = 0;

    int r_y = 0;
    int g_y = 0;
    int b_y = 0;

    int sobel_x[] = { -1, 0, 1, -2, 0, 2, -1, 0, 1 };
    int sobel_y[] = { -1, -2, -1, 0, 0, 0, 1, 2, 1 };

    for (int i = -1; i <= 1; i++) {
        for (int j = -1; j <= 1; j++) {
            int index = (pos * 4) + (i * 4) + (j * width * 4);

            if (index >= 0 && index <= width * height * 4) {
                int matrixIndex = (i + 1) * 3 + (j + 1);

                int weight_x = sobel_x[matrixIndex];
                int weight_y = sobel_y[matrixIndex];

                r_x += image[index] * weight_x;
                g_x += image[index + 1] * weight_x;
                b_x += image[index + 2] * weight_x;

                r_y += image[index] * weight_y;
                g_y += image[index + 1] * weight_y;
                b_y += image[index + 2] * weight_y;
            }
        }
    }

    int r = std::clamp((std::abs(r_x) + std::abs(r_y))/2, 0, 255);
    int g = std::clamp((std::abs(g_x) + std::abs(g_y))/2, 0, 255);
    int b = std::clamp((std::abs(b_x) + std::abs(b_y))/2, 0, 255);

    return { r, g, b };
}

void blur_image_sobel(const sf::Uint8 *image) {
    image_nueva.resize(image_width * image_height * image_channels);

#pragma omp parallel collapsed(2)
    for (int i = 0; i < image_width; i++) {
        for (int j = 0; j < image_height; j++) {

            auto [r, g, b] = process_pixel_sobel(image, image_width, image_height, j * image_width + i);

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
                    blur_image_edge(host_src_image_pixels);
                    auto end = ch::high_resolution_clock::now();
                    ch::duration<double, std::milli> tiempo = end - start;
                    last_time = tiempo.count();
                    //texture.create(image_width, image_height);
                    texture.update(image_nueva.data());
                }
                else if (event.key.scancode == sf::Keyboard::Scan::N) {
                    auto start = ch::high_resolution_clock::now();
                    blur_image_sobel(host_src_image_pixels);
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