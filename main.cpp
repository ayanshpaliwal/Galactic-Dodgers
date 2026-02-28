#include <SFML/Graphics.hpp>
#include <vector>
#include <string>
#include <cstdlib>
#include <ctime>
#include <optional>
#include <cmath>
#include <algorithm>

const int GRID_W = 10;
const int GRID_H = 20;
const int CELL_SIZE = 40; 
const unsigned int WINDOW_W = GRID_W * CELL_SIZE;
const unsigned int WINDOW_H = GRID_H * CELL_SIZE;

struct Entity {
    float x, y;
    float speed;
    int width = 1, height = 1;
    bool active = true;
    int type = 0; 
};

class GalacticDodger {
private:
    sf::RenderWindow window;
    sf::Font font;
    sf::Text uiText;
    
    int score = 0, highScore = 0, health = 100;
    bool gameOver = false;
    
    float playerX = 4.5f;
    const float playerY = GRID_H - 1; 
    
    float fireTimer = 0;
    float spawnTimer = 0;
    float obstacleTimer = 0;
    float powerUpTimer = 0; 
    
    std::vector<sf::Vector2f> stars;
    std::vector<Entity> playerBullets, enemyBullets, enemies, obstacles, powerUps;

public:
    GalacticDodger() 
        : window(sf::VideoMode({WINDOW_W, WINDOW_H}), "Galactic Dodger v3.0"),
          uiText(font) 
    {
        std::srand(static_cast<unsigned>(std::time(nullptr)));
        
        window.setFramerateLimit(20); 
        
        if (!font.openFromFile("arial.ttf")) { }
        uiText.setCharacterSize(18);
        uiText.setFillColor(sf::Color::White);

        for(int i=0; i<20; i++) stars.push_back({(float)(rand()%WINDOW_W), (float)(rand()%WINDOW_H)});
        initGame();
    }

    void initGame() {
        score = 0; health = 100; gameOver = false;
        playerX = 4.5f; powerUpTimer = 0;
        playerBullets.clear(); enemyBullets.clear(); enemies.clear(); obstacles.clear(); powerUps.clear();
    }

    void handleInput() {
        while (const std::optional event = window.pollEvent()) {
            if (event->is<sf::Event::Closed>()) window.close();
            if (const auto* keyPressed = event->getIf<sf::Event::KeyPressed>()) {
                if (gameOver && keyPressed->code == sf::Keyboard::Key::R) initGame();
            }
        }
        
        float moveSpeed = 0.4f; 
        if (sf::Keyboard::isKeyPressed(sf::Keyboard::Key::A) && playerX > 0) playerX -= moveSpeed;
        if (sf::Keyboard::isKeyPressed(sf::Keyboard::Key::D) && playerX < GRID_W - 1) playerX += moveSpeed;
    }

    void update(float dt) {
        float difficulty = 1.0f + (score / 200.0f);

        fireTimer += dt;
        if (fireTimer >= 0.35f) { 
            playerBullets.push_back({playerX + 0.3f, playerY, 12.0f});
            if (powerUpTimer > 0) {
                playerBullets.push_back({playerX + 0.3f, playerY, 12.0f, 1, 1, true, 1});
                playerBullets.push_back({playerX + 0.3f, playerY, 12.0f, 1, 1, true, 2});
            }
            fireTimer = 0;
        }

        spawnTimer += dt;
        if (spawnTimer >= (1.8f / difficulty)) {
            enemies.push_back({(float)(rand() % GRID_W), -1.0f, 3.0f * difficulty});
            spawnTimer = 0;
        }

        obstacleTimer += dt;
        if (obstacleTimer >= 6.0f) {
            obstacles.push_back({(float)(rand() % 8), -2.0f, 2.0f, 2, 2});
            obstacleTimer = 0;
        }

        if (powerUpTimer > 0) powerUpTimer -= dt;

        for (auto& b : playerBullets) {
            b.y -= b.speed * dt;
            if (b.type == 1) b.x -= 2.5f * dt;
            if (b.type == 2) b.x += 2.5f * dt;
        }
        for (auto& e : enemies) {
            e.y += e.speed * dt;
            if (rand() % 100 < 3) enemyBullets.push_back({e.x + 0.3f, e.y + 0.5f, 6.0f});
        }
        for (auto& eb : enemyBullets) eb.y += eb.speed * dt;
        for (auto& o : obstacles) o.y += o.speed * dt;
        for (auto& p : powerUps) p.y += p.speed * dt;

        for (auto& eb : enemyBullets) {
            if (eb.active && std::abs(playerX - eb.x) < 0.8f && std::abs(playerY - eb.y) < 0.8f) {
                eb.active = false; health -= 10;
            }
        }

        for (auto& o : obstacles) {
            if (o.active) {
                if (playerX + 0.8f >= o.x && playerX <= o.x + o.width - 0.2f &&
                    playerY + 0.8f >= o.y && playerY <= o.y + o.height - 0.2f) {
                    health = 0;
                }
            }
        }

        for (auto& b : playerBullets) {
            for (auto& e : enemies) {
                if (b.active && e.active && std::abs(b.x - e.x) < 0.9f && std::abs(b.y - e.y) < 0.9f) {
                    b.active = false; e.active = false; score += 10;
                    if (rand() % 8 == 0) powerUps.push_back({e.x, e.y, 3.0f}); 
                }
            }
        }

        for (auto& p : powerUps) {
            if (p.active && std::abs(playerX - p.x) < 0.9f && std::abs(playerY - p.y) < 0.9f) {
                p.active = false; powerUpTimer = 7.0f;
            }
        }

        for (auto& e : enemies) {
            if (e.active && e.y >= GRID_H) { e.active = false; score -= 5; }
        }

        auto isOff = [](const Entity& e) { return !e.active || e.y < -4 || e.y > GRID_H + 1; };
        enemies.erase(std::remove_if(enemies.begin(), enemies.end(), isOff), enemies.end());
        playerBullets.erase(std::remove_if(playerBullets.begin(), playerBullets.end(), isOff), playerBullets.end());
        enemyBullets.erase(std::remove_if(enemyBullets.begin(), enemyBullets.end(), isOff), enemyBullets.end());
        obstacles.erase(std::remove_if(obstacles.begin(), obstacles.end(), isOff), obstacles.end());
        powerUps.erase(std::remove_if(powerUps.begin(), powerUps.end(), isOff), powerUps.end());

        if (health <= 0) { health = 0; gameOver = true; if (score > highScore) highScore = score; }
    }

    void draw() {
        window.clear(sf::Color(5, 5, 20));
        sf::RectangleShape rect;

        rect.setFillColor(sf::Color(25, 25, 40));
        for(int x=0; x<=GRID_W; x++) {
            rect.setSize({1, (float)WINDOW_H});
            rect.setPosition({(float)x * CELL_SIZE, 0});
            window.draw(rect);
        }

        for (const auto& o : obstacles) {
            rect.setFillColor(sf::Color(90, 80, 70));
            rect.setSize({(float)o.width * CELL_SIZE - 4, (float)o.height * CELL_SIZE - 4});
            rect.setPosition({o.x * CELL_SIZE + 2, o.y * CELL_SIZE + 2});
            window.draw(rect);
        }

        rect.setSize({CELL_SIZE - 8, CELL_SIZE - 8});
        for (const auto& e : enemies) {
            rect.setFillColor(sf::Color::Red);
            rect.setPosition({e.x * CELL_SIZE + 4, e.y * CELL_SIZE + 4});
            window.draw(rect);
        }

        for (const auto& p : powerUps) {
            sf::CircleShape pOrb(CELL_SIZE / 3.0f);
            pOrb.setFillColor(sf::Color::Yellow);
            pOrb.setPosition({p.x * CELL_SIZE + 8, p.y * CELL_SIZE + 8});
            window.draw(pOrb);
        }

        rect.setSize({8, 16});
        for (const auto& b : playerBullets) {
            rect.setFillColor(sf::Color::Cyan);
            rect.setPosition({b.x * CELL_SIZE + 16, b.y * CELL_SIZE});
            window.draw(rect);
        }
        for (const auto& eb : enemyBullets) {
            rect.setFillColor(sf::Color::Magenta);
            rect.setPosition({eb.x * CELL_SIZE + 16, eb.y * CELL_SIZE});
            window.draw(rect);
        }

        rect.setFillColor(powerUpTimer > 0 ? sf::Color::Yellow : sf::Color::Green);
        rect.setSize({CELL_SIZE - 4, CELL_SIZE - 4});
        rect.setPosition({playerX * CELL_SIZE + 2, playerY * CELL_SIZE + 2});
        window.draw(rect);

        uiText.setString("SCORE: " + std::to_string(score) + "  HP: " + std::to_string(health) + "%");
        uiText.setPosition({10, 10});
        window.draw(uiText);

        if (gameOver) {
            uiText.setCharacterSize(32);
            uiText.setString("SYSTEM FAILURE\nPress R to Reboot");
            uiText.setPosition({40, WINDOW_H / 2 - 50});
            window.draw(uiText);
            uiText.setCharacterSize(18);
        }
        window.display();
    }

    void run() {
        sf::Clock clock;
        while (window.isOpen()) {
            float dt = clock.restart().asSeconds();
            handleInput();
            if (!gameOver) update(dt);
            draw();
        }
    }
};

int main() {
    GalacticDodger game;
    game.run();
    return 0;
}