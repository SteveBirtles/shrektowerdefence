#define OLC_PGE_APPLICATION
#include <fstream>
#include <iostream>
#include <string>
#include <vector>

#include "olcPixelGameEngine.h"

const int WINDOW_WIDTH = 1280;
const int WINDOW_HEIGHT = 720;

const int TILE_SIZE = 32;
const int TILE_COUNT = 26;
olc::Sprite *tileSprites[TILE_COUNT];
olc::Decal *tileDecals[TILE_COUNT];
const std::string tileFiles[TILE_COUNT]{
    "bush.png",   "FarFarAway.png",  "Grass1.png", "Grass2.png", "Grass3.png",
    "Grass4.png", "Grass5.png",      "Grass6.png", "House.png",  "Marsh.png",
    "Mud.png",    "Onion_field.png", "soil2.png",  "soil.png",   "stone.png",
    "swamp.png",  "Toilet.png",      "tree.png",   "Water2.png", "Water3.png",
    "Water4.png", "Water5.png",      "Water6.png", "Water7.png", "Water8.png",
    "Water.png",
};

void loadTileGraphics() {
  for (auto i = 0; i < TILE_COUNT; i++) {
    tileSprites[i] = new olc::Sprite("./Tiles/" + tileFiles[i]);
    tileDecals[i] = new olc::Decal(tileSprites[i]);
  }
}

const int MAP_WIDTH = 40;
const int MAP_HEIGHT = 20;
int map[MAP_WIDTH][MAP_HEIGHT];

const int TOWER_SIZE = 32;
const int TOWER_COUNT = 7;
olc::Sprite *towerSprites[TOWER_COUNT];
olc::Decal *towerDecals[TOWER_COUNT];
const std::string towerFiles[TOWER_COUNT]{
    "donkey.png",        "Dragon.png",          "gingerbreadSpriteSheet.png",
    "Puss_in_boots.png", "rumpelstiltskin.png", "shrek.png",
    "smashMouth.png",
};

struct Tower {
  int x;
  int y;
  int towerNumber;
};
std::vector<Tower *> towers;

void loadTowerGraphics() {
  for (auto i = 0; i < TOWER_COUNT; i++) {
    towerSprites[i] = new olc::Sprite("./Towers/" + towerFiles[i]);
    towerDecals[i] = new olc::Decal(towerSprites[i]);
  }
}

void addRandomTowers() {
  for (auto i = 0; i < 40; i++) {
    if (towers.size() == MAP_WIDTH * MAP_HEIGHT) break;
    int x, y;
    auto repeated = true;
    while (repeated) {
      x = rand() % MAP_WIDTH;
      y = rand() % MAP_HEIGHT;
      repeated = false;
      for (auto tower : towers) {
        if (tower->x == x && tower->y == y) {
          repeated = true;
        }
      }
    }
    towers.push_back(new Tower{x, y, rand() % TOWER_COUNT});
  }
}

void resetTowers() {
  for (auto tower : towers) {
    delete tower;
  }
  towers.clear();
}

void resetMap() {
  for (auto x = 0; x < MAP_WIDTH; x++) {
    for (auto y = 0; y < MAP_HEIGHT; y++) {
      map[x][y] = 10;  // Mud.png
    }
  }
}

void saveMap() {
  std::ofstream mapFile;
  mapFile.open("map.csv");
  mapFile << "MAP_WIDTH," << std::to_string(MAP_WIDTH) << std::endl;
  mapFile << "MAP_HEIGHT," << std::to_string(MAP_HEIGHT) << std::endl;
  for (auto y = 0; y < MAP_HEIGHT; y++) {
    mapFile << "MAP_ROW,";
    for (auto x = 0; x < MAP_WIDTH; x++) {
      mapFile << std::to_string(map[x][y]);
      if (x != MAP_WIDTH - 1) {
        mapFile << ",";
      } else {
        mapFile << std::endl;
      }
    }
  }
  for (auto t = 0; t < towers.size(); t++) {
    mapFile << "TOWER," << std::to_string(towers[t]->x) << ","
            << std::to_string(towers[t]->y) << ","
            << std::to_string(towers[t]->towerNumber) << std::endl;
  }
  mapFile.close();
}

void loadMap() {
  std::string line;
  std::ifstream mapFile("map.csv");

  resetTowers();

  if (mapFile.is_open()) {
    int x = 0;
    int y = -1;
    std::string bit;
    enum RowType { UNKNOWN, WIDTH_CHECK, HEIGHT_CHECK, MAP_ROW, TOWER };
    Tower *newTower;

    while (std::getline(mapFile, line)) {
      try {
        std::size_t commaPosition = line.find(",");
        std::size_t lastCommaPosition = -1;
        bool done = false;
        auto rowType = UNKNOWN;

        while (!done) {
          if (commaPosition == std::string::npos) {
            bit = line.substr(lastCommaPosition + 1);
            done = true;
          } else {
            bit = line.substr(lastCommaPosition + 1,
                              commaPosition - lastCommaPosition - 1);
            lastCommaPosition = commaPosition;
            commaPosition = line.find(",", lastCommaPosition + 1);
          }

          if (bit == "MAP_WIDTH") {
            rowType = WIDTH_CHECK;
          } else if (bit == "MAP_HEIGHT") {
            rowType = HEIGHT_CHECK;
          } else if (bit == "MAP_ROW") {
            rowType = MAP_ROW;
            x = 0;
            y++;
          } else if (bit == "TOWER") {
            rowType = TOWER;
            newTower = new Tower();
            x = 0;
          } else {
            switch (rowType) {
              case WIDTH_CHECK:
                if (std::stoi(bit) != MAP_WIDTH) {
                  std::cerr << "Tried to load a map with invalid width."
                            << std::endl;
                  return;
                }
                break;
              case HEIGHT_CHECK:
                if (std::stoi(bit) != MAP_HEIGHT) {
                  std::cerr << "Tried to load a map with invalid height."
                            << std::endl;
                  return;
                }
                break;
              case MAP_ROW:
                map[x][y] = std::stoi(bit);
                x++;
                break;
              case TOWER:
                if (x == 0) {
                  newTower->x = std::stoi(bit);
                } else if (x == 1) {
                  newTower->y = std::stoi(bit);
                } else if (x == 2) {
                  newTower->towerNumber = std::stoi(bit);
                  towers.push_back(newTower);
                }
                x++;
            }
          }
        }
      } catch (int e) {
        std::cout << "An error occured: " << e << std::endl;
      }
    }
  }
}

const int ANIMATION_RATE = 6;

class Game : public olc::PixelGameEngine {
 public:
  Game() { sAppName = "Shrek Tower Defence"; }

 private:
  float timer = 0;
  int frames = 0;
  int fps;
  int currentTile = 2;  // Grass1.png
  float animationTimer = 0;

 public:
  bool OnUserCreate() override {
    loadTileGraphics();
    loadTowerGraphics();
    resetMap();
    resetTowers();
    addRandomTowers();

    return true;
  }

  bool OnUserUpdate(float fElapsedTime) override {
    timer += fElapsedTime;
    frames++;
    if (timer > 1.0) {
      fps = frames;
      frames = 0;
      timer -= 1;
    }
    animationTimer += fElapsedTime;

    inputs();
    outputs();

    if (GetKey(olc::Key::ESCAPE).bPressed) {
      return false;
    } else {
      return true;
    }
  }

  void inputs() {
    if (GetKey(olc::Key::DOWN).bPressed) {
      currentTile = (currentTile + 1) % TILE_COUNT;
    }

    if (GetKey(olc::Key::UP).bPressed) {
      currentTile = (currentTile + TILE_COUNT - 1) % TILE_COUNT;
    }

    if (GetKey(olc::Key::N).bPressed) {
      resetMap();
      resetTowers();
    }

    if (GetKey(olc::Key::S).bPressed) {
      saveMap();
    }

    if (GetKey(olc::Key::L).bPressed) {
      loadMap();
    }

    if (GetKey(olc::Key::T).bPressed) {
      addRandomTowers();
    }

    if (GetMouse(0).bHeld || GetMouse(1).bHeld) {
      auto x = GetMouseX() / TILE_SIZE;
      auto y = GetMouseY() / TILE_SIZE;
      if (x < 0) x = 0;
      if (x > MAP_WIDTH - 1) x = MAP_WIDTH - 1;
      if (y < 0) y = 0;
      if (y > MAP_HEIGHT - 1) y = MAP_HEIGHT - 1;
      if (GetMouse(0).bHeld) {
        map[x][y] = currentTile;
      } else {
        currentTile = map[x][y];
      }
    }
  }

  void outputs() {
    SetPixelMode(olc::Pixel::NORMAL);

    for (auto x = 0; x < MAP_WIDTH; x++) {
      for (auto y = 0; y < MAP_HEIGHT; y++) {
        auto index = map[x][y];
        auto position = olc::vi2d{x * TILE_SIZE, y * TILE_SIZE};
        DrawDecal(position, tileDecals[index]);
      }
    }

    auto position = olc::vi2d{20, WINDOW_HEIGHT - TILE_SIZE - 20};
    DrawDecal(position, tileDecals[currentTile]);

    SetPixelMode(olc::Pixel::MASK);

    for (auto tower : towers) {
      auto index = tower->towerNumber;
      auto frameCount = towerSprites[index]->width / TOWER_SIZE;
      auto frame = int(animationTimer * ANIMATION_RATE) % frameCount;
      auto position = olc::vi2d{tower->x * TILE_SIZE, tower->y * TILE_SIZE};
      auto size = olc::vi2d{TOWER_SIZE, TOWER_SIZE};
      auto offset = olc::vi2d{TOWER_SIZE * frame, 0};
      DrawPartialDecal(position, size, towerDecals[index], offset, size);
    }

    if (fps > 0) {
      auto position = olc::vi2d(WINDOW_WIDTH - 70, WINDOW_HEIGHT - 70);
      DrawStringDecal(position, "FPS " + std::to_string(fps));
    }
  }

  bool OnUserDestroy() override {
    std::cout << "Closing application" << std::endl;
    return true;
  }
};

int main() {
  Game game;
  if (game.Construct(WINDOW_WIDTH, WINDOW_HEIGHT, 1, 1)) game.Start();
  return 0;
}
