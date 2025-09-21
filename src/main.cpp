#include <SFML/Graphics.hpp>
#include <optional>

int main()
{
    sf::RenderWindow window(sf::VideoMode({800, 600}), "SFML 3 on Apple Silicon");
    window.setFramerateLimit(60);

    sf::CircleShape shape(80.f);
    shape.setFillColor(sf::Color::Green);
    shape.setPosition(sf::Vector2f(400.f, 300.f));

    while (window.isOpen())
    {
        // В SFML 3 pollEvent() возвращает std::optional<Event>,
        // и проверка типа делается через event->is<T>()
        while (auto event = window.pollEvent())
        {
            if (event->is<sf::Event::Closed>())
            {
                window.close();
            }
            if (event->is<sf::Event::KeyPressed>())
            {
                // ESC — выходим
                if (event->getIf<sf::Event::KeyPressed>()->scancode == sf::Keyboard::Scan::Escape)
                    window.close();
            }
        }

        window.clear();
        window.draw(shape);
        window.display();
    }
}
