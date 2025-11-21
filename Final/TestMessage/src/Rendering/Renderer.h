#pragma once
#include <C:\Users\killi\source\SFML-2.6.1\include\SFML\Graphics.hpp>
#include "../Entity/Entity.h"
#include "../Core/Utils.h"
#include <vector>

// Classe pour renderer la simulation SFML
class Renderer {
private:
    sf::RenderWindow mWindow;
    sf::Font mFont;
    bool mFontLoaded;

    // Couleurs par thread
    sf::Color mThreadColors[8] = {
        sf::Color(255, 100, 100),  // Rouge (Thread 0)
        sf::Color(100, 255, 100),  // Vert (Thread 1)
        sf::Color(100, 100, 255),  // Bleu (Thread 2)
        sf::Color(255, 255, 100),  // Jaune (Thread 3)
        sf::Color(255, 100, 255),  // Magenta (Thread 4)
        sf::Color(100, 255, 255),  // Cyan (Thread 5)
        sf::Color(255, 200, 100),  // Orange (Thread 6)
        sf::Color(200, 100, 255)   // Violet (Thread 7)
    };

public:
    Renderer() : mFontLoaded(false) {
        // Créer fenêtre
        mWindow.create(
            sf::VideoMode((unsigned int)Config::MAP_WIDTH, (unsigned int)Config::MAP_HEIGHT),
            "Multithreading Collision Test - SFML"
        );
        mWindow.setFramerateLimit(60);

        // Essayer charger font (optionnel)
        // Si pas de font, on affiche quand même sans texte
    }

    bool IsOpen() const {
        return mWindow.isOpen();
    }

    void ProcessEvents() {
        sf::Event event;
        while (mWindow.pollEvent(event)) {
            if (event.type == sf::Event::Closed) {
                mWindow.close();
            }
            if (event.type == sf::Event::KeyPressed) {
                if (event.key.code == sf::Keyboard::Escape) {
                    mWindow.close();
                }
            }
        }
    }

    void Clear() {
        mWindow.clear(sf::Color(30, 30, 40));  // Fond sombre
    }

    void DrawGrid() {
        sf::RectangleShape line;

        // Lignes verticales
        for (int x = 0; x <= Config::MAP_WIDTH; x += (int)Config::CELL_SIZE) {
            line.setSize(sf::Vector2f(1, Config::MAP_HEIGHT));
            line.setPosition((float)x, 0);
            line.setFillColor(sf::Color(60, 60, 70, 100));  // Gris transparent
            mWindow.draw(line);
        }

        // Lignes horizontales
        for (int y = 0; y <= Config::MAP_HEIGHT; y += (int)Config::CELL_SIZE) {
            line.setSize(sf::Vector2f(Config::MAP_WIDTH, 1));
            line.setPosition(0, (float)y);
            line.setFillColor(sf::Color(60, 60, 70, 100));
            mWindow.draw(line);
        }
    }

    void DrawBall(Entity* entity) {
        if (!entity || entity->mIsDead) return;

        // Cercle
        sf::CircleShape circle(entity->mRadius);
        circle.setPosition(entity->mPos.x - entity->mRadius, entity->mPos.y - entity->mRadius);

        // Couleur selon thread
        int threadID = entity->mThreadIndex;
        if (threadID >= 0 && threadID < 8) {
            circle.setFillColor(mThreadColors[threadID]);
        } else {
            circle.setFillColor(sf::Color::White);
        }

        // Contour noir
        circle.setOutlineThickness(2);
        circle.setOutlineColor(sf::Color::Black);

        // Transparence si en transition
        if (entity->mIsTransitioning) {
            sf::Color col = circle.getFillColor();
            col.a = 128;  // Semi-transparent
            circle.setFillColor(col);
        }

        mWindow.draw(circle);

        // Direction (petite ligne)
        sf::Vertex line[] = {
            sf::Vertex(sf::Vector2f(entity->mPos.x, entity->mPos.y)),
            sf::Vertex(sf::Vector2f(
                entity->mPos.x + entity->mDirection.x * entity->mRadius * 1.5f,
                entity->mPos.y + entity->mDirection.y * entity->mRadius * 1.5f
            ))
        };
        line[0].color = sf::Color::White;
        line[1].color = sf::Color::White;
        mWindow.draw(line, 2, sf::Lines);
    }

    void DrawHUD(int aliveCount, int totalCollisions, int totalMessages) {
        // Texte simple (sans font pour éviter erreurs)
        // On pourrait ajouter sf::Text si on charge une font

        // Pour l'instant, juste afficher stats dans la console
        // (le texte SFML nécessite une font)
    }

    void Display() {
        mWindow.display();
    }
};
