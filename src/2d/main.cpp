
#include <SFML/Graphics.hpp>
#include <optional>
#include <vector>
#include <cmath>

#include "ik_math/ik_solver.hpp"

static void drawJoint(sf::RenderTarget &rt, sf::Vector2f p,
                      float r = 6.f, sf::Color fill = sf::Color(240, 240, 240))
{
    sf::CircleShape c(r);
    c.setOrigin({r, r});
    c.setPosition(p);
    c.setFillColor(fill);
    c.setOutlineColor(sf::Color::Black);
    c.setOutlineThickness(1.f);
    rt.draw(c);
}

static void drawBone(sf::RenderTarget &rt, sf::Vector2f a, sf::Vector2f b,
                     float thick = 8.f, sf::Color col = sf::Color(90, 140, 250))
{
    const sf::Vector2f d = b - a;
    const float len = d.length();
    if (len <= 1e-6f)
        return;

    sf::RectangleShape r({len, thick});
    r.setOrigin({0.f, thick / 2.f});
    r.setPosition(a);
    r.setRotation(d.angle());
    r.setFillColor(col);
    rt.draw(r);
}

static void drawReachCircle(sf::RenderTarget &rt, sf::Vector2f center, float radius)
{
    sf::CircleShape c(radius);
    c.setOrigin({radius, radius});
    c.setPosition(center);
    c.setFillColor(sf::Color(0, 0, 0, 0));
    c.setOutlineColor(sf::Color(200, 200, 200, 140));
    c.setOutlineThickness(1.5f);
    rt.draw(c);
}

void draw2d()
{
    sf::ContextSettings settings;
    sf::RenderWindow window(sf::VideoMode{sf::Vector2u{1000u, 700u}}, "IK_Solver FABRIK (SFML 3)");
    window.setFramerateLimit(120);

    const sf::Vector2f root{300.f, 350.f};
    const int N = 8;
    const float segLen = 60.f;

    std::vector<sf::Vector2f> joints(N);
    for (int i = 0; i < N; ++i)
        joints[i] = root + sf::Vector2f{segLen * i, 0.f};

    std::vector<float> L(N - 1, segLen);

    sf::Vector2f target{700.f, 350.f};
    bool followMouse = true;

    IKSolver solver;

    while (window.isOpen())
    {
        while (const std::optional ev = window.pollEvent())
        {
            if (ev->is<sf::Event::Closed>())
                window.close();

            else if (const auto *key = ev->getIf<sf::Event::KeyPressed>())
            {
                if (key->scancode == sf::Keyboard::Scancode::Escape)
                    window.close();
                if (key->scancode == sf::Keyboard::Scancode::Space)
                    followMouse = !followMouse;
                if (key->scancode == sf::Keyboard::Scancode::R)
                {
                    for (int i = 0; i < N; ++i)
                        joints[i] = root + sf::Vector2f{segLen * i, 0.f};
                }
            }
            else if (const auto *mb = ev->getIf<sf::Event::MouseButtonPressed>())
            {
                if (mb->button == sf::Mouse::Button::Left)
                    target = sf::Vector2f(mb->position);
            }
            else if (const auto *mm = ev->getIf<sf::Event::MouseMoved>())
            {
                if (followMouse && sf::Mouse::isButtonPressed(sf::Mouse::Button::Left))
                    target = sf::Vector2f(mm->position);
            }
        }

        solver.fabrikSolve(joints, L, root, target, 24, 1e-3f);

        window.clear(sf::Color(25, 28, 34));

        float totalLen = 0.f;
        for (float li : L)
            totalLen += li;
        drawReachCircle(window, root, totalLen);

        sf::CircleShape t(7.f);
        t.setOrigin({7.f, 7.f});
        t.setPosition(target);
        t.setFillColor(sf::Color(255, 90, 90));
        t.setOutlineColor(sf::Color::Black);
        t.setOutlineThickness(1.f);
        window.draw(t);

        for (int i = 0; i < N - 1; ++i)
            drawBone(window, joints[i], joints[i + 1]);
        for (auto &p : joints)
            drawJoint(window, p);
        drawJoint(window, root, 7.f, sf::Color(255, 215, 0));

        window.display();
    }
}

int main()
{
    draw2d();
}
