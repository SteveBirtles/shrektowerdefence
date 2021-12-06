#define OLC_PGE_APPLICATION
#include "olcPixelGameEngine.h"

const int WINDOW_WIDTH = 1280;
const int WINDOW_HEIGHT = 720;

const int TILE_SIZE = 32;
const int TILE_COUNT = 26;

const int MAP_WIDTH = 40;
const int MAP_HEIGHT = 20;

class Game : public olc::PixelGameEngine {
 public:
  Game() { sAppName = "Insert Game Name Here"; }

 private:
  float timer = 0;
  int frames = 0;
  int fps;

  olc::Sprite* tileSprites[TILE_COUNT];
  olc::Decal* tileDecals[TILE_COUNT];

  int currentTile = 2;  // Grass1.png

  const std::string tileFiles[TILE_COUNT]{
      "bush.png",   "FarFarAway.png",  "Grass1.png", "Grass2.png", "Grass3.png",
      "Grass4.png", "Grass5.png",      "Grass6.png", "House.png",  "Marsh.png",
      "Mud.png",    "Onion_field.png", "soil2.png",  "soil.png",   "stone.png",
      "swamp.png",  "Toilet.png",      "tree.png",   "Water2.png", "Water3.png",
      "Water4.png", "Water5.png",      "Water6.png", "Water7.png", "Water8.png",
      "Water.png",
  };

  int map[MAP_WIDTH][MAP_HEIGHT];

  void loadTileGraphics() {
    for (auto i = 0; i < TILE_COUNT; i++) {
      tileSprites[i] = new olc::Sprite("./Tiles/" + tileFiles[i]);
      tileDecals[i] = new olc::Decal(tileSprites[i]);
    }
  }

  void resetMap() {
    for (auto x = 0; x < MAP_WIDTH; x++) {
      for (auto y = 0; y < MAP_HEIGHT; y++) {
        map[x][y] = 10;  // Mud.png
      }
    }
  }

 public:
  bool OnUserCreate() override {
    /*
      Load resources here
    */
    loadTileGraphics();
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
    processes();
    outputs();

    if (GetKey(olc::Key::ESCAPE).bPressed) {
      return false;
    } else {
      return true;
    }
  }

  void inputs() {
    /*
      Game controls goes here
    */

    if (GetKey(olc::Key::DOWN).bPressed) {
      currentTile++;
      if (currentTile > TILE_COUNT - 1) currentTile = 0;
    }

    if (GetKey(olc::Key::UP).bPressed) {
      currentTile--;
      if (currentTile < 0) currentTile = TILE_COUNT - 1;
    }

    auto x = GetMouseX() / TILE_SIZE;
    auto y = GetMouseY() / TILE_SIZE;

    if (x < 0) x = 0;
    if (x > MAP_WIDTH - 1) x = MAP_WIDTH - 1;
    if (y < 0) y = 0;
    if (y > MAP_HEIGHT - 1) y = MAP_HEIGHT - 1;

    if (GetMouse(0).bHeld) {
      map[x][y] = currentTile;
    } else if (GetMouse(1).bHeld) {
      currentTile = map[x][y];
    }
  }

  void processes() {
    /*
      Game logic goes here
    */
  }

  void outputs() {
    SetPixelMode(olc::Pixel::NORMAL);

    /*
      Game graphics drawn here
    */
    for (auto x = 0; x < MAP_WIDTH; x++) {
      for (auto y = 0; y < MAP_HEIGHT; y++) {
        auto index = map[x][y];
        auto position = olc::vi2d{x * TILE_SIZE, y * TILE_SIZE};
        DrawDecal(position, tileDecals[index]);
      }
    }

    auto position = olc::vi2d{20, WINDOW_HEIGHT - TILE_SIZE - 20};
    DrawDecal(position, tileDecals[currentTile]);

    if (fps > 0) {
      auto fpsPosition = olc::vi2d(WINDOW_WIDTH - 70, WINDOW_HEIGHT - 70);
      DrawStringDecal(fpsPosition, "FPS " + std::to_string(fps));
    }
  }

  bool OnUserDestroy() override {
    std::cout << "Closing game" << std::endl;
    for (auto i = 0; i < TILE_COUNT; i++) {
      delete tileSprites[i];
      delete tileDecals[i];
    }
    return true;
  }
};

int main() {
  Game game;
  if (game.Construct(WINDOW_WIDTH, WINDOW_HEIGHT, 1, 1)) game.Start();
  return 0;
}
