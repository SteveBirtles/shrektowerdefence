#define OLC_PGE_APPLICATION
#include <string>

#include "olcPixelGameEngine.h"

const int WINDOW_WIDTH = 1280;
const int WINDOW_HEIGHT = 720;

const int TILE_SIZE = 32;
const int TILE_COUNT = 26;
olc::Sprite* tileSprites[TILE_COUNT];
olc::Decal* tileDecals[TILE_COUNT];
const std::string tileFiles[TILE_COUNT]{
    "bush.png",   "FarFarAway.png",  "Grass1.png", "Grass2.png", "Grass3.png",
    "Grass4.png", "Grass5.png",      "Grass6.png", "House.png",  "Marsh.png",
    "Mud.png",    "Onion_field.png", "soil2.png",  "soil.png",   "stone.png",
    "swamp.png",  "Toilet.png",      "tree.png",   "Water2.png", "Water3.png",
    "Water4.png", "Water5.png",      "Water6.png", "Water7.png", "Water8.png",
    "Water.png",
};

void loadTiles() {
  for (auto i = 0; i < TILE_COUNT; i++) {
    tileSprites[i] = new olc::Sprite("./Tiles/" + tileFiles[i]);
    tileDecals[i] = new olc::Decal(tileSprites[i]);
  }
}

const int TOWER_SIZE = 32;
const int TOWER_COUNT = 7;
olc::Sprite* towerSprites[TOWER_COUNT];
olc::Decal* towerDecals[TOWER_COUNT];
const std::string towerFiles[TOWER_COUNT]{
    "donkey.png",        "Dragon.png",          "gingerbreadSpriteSheet.png",
    "Puss_in_boots.png", "rumpelstiltskin.png", "shrek.png",
    "smashMouth.png",
};

void loadTowers() {
  for (auto i = 0; i < TOWER_COUNT; i++) {
    towerSprites[i] = new olc::Sprite("./Towers/" + towerFiles[i]);
    towerDecals[i] = new olc::Decal(towerSprites[i]);
  }
}

const int MAP_WIDTH = 40;
const int MAP_HEIGHT = 20;
int map[MAP_WIDTH][MAP_HEIGHT];

void resetMap() {
  for (auto x = 0; x < MAP_WIDTH; x++) {
    for (auto y = 0; y < MAP_HEIGHT; y++) {
      map[x][y] = 10;  // Mud.png
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
    loadTiles();
    loadTowers();
    resetMap();
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

    animationTimer += GetElapsedTime();
    for (auto t = 0; t < TOWER_COUNT; t++) {
      auto frameCount = towerSprites[t]->width / TOWER_SIZE;
      auto frame = int(animationTimer * ANIMATION_RATE) % frameCount;
      auto position = olc::vi2d{100 + t * 50, WINDOW_HEIGHT - 50};
      auto size = olc::vi2d{TOWER_SIZE, TOWER_SIZE};
      auto offset = olc::vi2d{TOWER_SIZE * frame, 0};

      DrawPartialDecal(position, size, towerDecals[t], offset, size);

      auto topLeft = position + olc::vi2d{-2, -2};
      auto topRight = position + olc::vi2d{TOWER_SIZE + 4, -2};
      auto bottomLeft = position + olc::vi2d{-2, TOWER_SIZE + 4};
      auto bottomRight = position + olc::vi2d{TOWER_SIZE + 4, TOWER_SIZE + 4};

      DrawLineDecal(topLeft, topRight);
      DrawLineDecal(topRight, bottomRight);
      DrawLineDecal(bottomRight, bottomLeft);
      DrawLineDecal(bottomLeft, topLeft);
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
