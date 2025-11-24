#pragma once
#include <C:\Users\killi\source\SFML-2.6.1\include\SFML\Graphics.hpp>
#include "../Entity/Entity.h"
#include "../Entity/Player.h"
#include "../Entity/Bonus.h"
#include "../Core/Utils.h"
#include <vector>
class Renderer {
private:
    sf::RenderWindow mWindow;
    sf::Font mFont;
    bool mFontLoaded;
    sf::Color mThreadColors[8] = {
        sf::Color(255, 100, 100),  
        sf::Color(100, 255, 100),  
        sf::Color(100, 100, 255),  
        sf::Color(255, 255, 100),  
        sf::Color(255, 100, 255),  
        sf::Color(100, 255, 255),  
        sf::Color(255, 200, 100),  
        sf::Color(200, 100, 255)   
    };
    sf::Color mBonusColors[4] = {
        sf::Color(255, 50, 50),    
        sf::Color(50, 150, 255),   
        sf::Color(255, 220, 50),   
        sf::Color(200, 50, 255)    
    };
public:
    Renderer() : mFontLoaded(false) {
        mWindow.create(
            sf::VideoMode((unsigned int)Config::MAP_WIDTH, (unsigned int)Config::MAP_HEIGHT),
            "TestComplet - Multithreading Game"
        );
        mWindow.setFramerateLimit(60);
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
        mWindow.clear(sf::Color(30, 30, 40));  
    }
    void DrawGrid() {
        sf::RectangleShape line;
        for (int x = 0; x <= Config::MAP_WIDTH; x += (int)Config::CELL_SIZE) {
            line.setSize(sf::Vector2f(1, Config::MAP_HEIGHT));
            line.setPosition((float)x, 0);
            line.setFillColor(sf::Color(60, 60, 70, 100));  
            mWindow.draw(line);
        }
        for (int y = 0; y <= Config::MAP_HEIGHT; y += (int)Config::CELL_SIZE) {
            line.setSize(sf::Vector2f(Config::MAP_WIDTH, 1));
            line.setPosition(0, (float)y);
            line.setFillColor(sf::Color(60, 60, 70, 100));
            mWindow.draw(line);
        }
    }
    void DrawEntity(Entity* entity) {
        if (!entity || entity->mIsDead) return;
        Player* player = dynamic_cast<Player*>(entity);
        Bonus* bonus = dynamic_cast<Bonus*>(entity);
        if (player) {
            DrawPlayer(player);
        } else if (bonus) {
            DrawBonus(bonus);
        }
    }
    void DrawPlayer(Player* player) {
        if (!player || player->mIsDead) return;
        if (player->mHasShield) {
            sf::CircleShape shield(player->mRadius + 5.0f);
            shield.setPosition(player->mPos.x - player->mRadius - 5.0f,
                             player->mPos.y - player->mRadius - 5.0f);
            shield.setFillColor(sf::Color::Transparent);
            shield.setOutlineThickness(3);
            shield.setOutlineColor(sf::Color(255, 220, 50, 180));  
            mWindow.draw(shield);
        }
        sf::CircleShape circle(player->mRadius);
        circle.setPosition(player->mPos.x - player->mRadius, player->mPos.y - player->mRadius);
        int threadID = player->mThreadIndex;
        if (threadID >= 0 && threadID < 8) {
            circle.setFillColor(mThreadColors[threadID]);
        } else {
            circle.setFillColor(sf::Color::White);
        }
        circle.setOutlineThickness(2);
        circle.setOutlineColor(sf::Color::Black);
        if (player->mIsTransitioning) {
            sf::Color col = circle.getFillColor();
            col.a = 128;  
            circle.setFillColor(col);
        }
        mWindow.draw(circle);
        sf::Vertex line[] = {
            sf::Vertex(sf::Vector2f(player->mPos.x, player->mPos.y)),
            sf::Vertex(sf::Vector2f(
                player->mPos.x + player->mDirection.x * player->mRadius * 1.5f,
                player->mPos.y + player->mDirection.y * player->mRadius * 1.5f
            ))
        };
        line[0].color = sf::Color::White;
        line[1].color = sf::Color::White;
        mWindow.draw(line, 2, sf::Lines);
    }
    void DrawBonus(Bonus* bonus) {
        if (!bonus || bonus->mIsDead) return;
        sf::CircleShape shape(bonus->mRadius, 6);  
        shape.setPosition(bonus->mPos.x - bonus->mRadius, bonus->mPos.y - bonus->mRadius);
        int typeIndex = (int)bonus->mType;
        if (typeIndex >= 0 && typeIndex < 4) {
            shape.setFillColor(mBonusColors[typeIndex]);
        } else {
            shape.setFillColor(sf::Color::White);
        }
        shape.setOutlineThickness(2);
        shape.setOutlineColor(sf::Color::White);
        mWindow.draw(shape);
    }
    void DrawHUD(int aliveCount, int totalCollisions, int totalMessages, int bonusPickups) {
    }
    void Display() {
        mWindow.display();
    }
};
