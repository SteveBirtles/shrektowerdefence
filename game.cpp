#define OLC_PGE_APPLICATION
#include "olcPixelGameEngine.h"

const int WINDOW_WIDTH = 1280;
const int WINDOW_HEIGHT = 720;

const int TILE_SIZE = 32;
const int TILE_COUNT = 26;

const int MAP_WIDTH = 40;
const int MAP_HEIGHT = 20;

const int MOB_SIZE = 32;
const int MOB_COUNT = 6;

class Game : public olc::PixelGameEngine {
 public:
  Game() { sAppName = "Insert Game Name Here"; }

 private:
  struct Mob {
    int type;
    int x;
    int y;
    int direction;
    int nextX;
    int nextY;
    float progress;
    float speed;
    float frame;
    Mob(int givenType, int startX, int startY) {
      x = startX;
      y = startY;
      type = givenType;
      direction = 'E';
      nextX = x + 1;
      nextY = y;
      progress = 0;
      speed = 4;
      frame = 0;
    }
  };

  std::vector<Mob> mobs;

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

  olc::Sprite* mobSprites[MOB_COUNT];
  olc::Decal* mobDecals[MOB_COUNT];

  const std::string mobFiles[MOB_COUNT]{
      "DoTheRoarKid.png", "FairyGodmother.png", "knight.png",
      "lordfarquaad.png", "Pied piper.png",     "Pied piper_s mice.png",
  };

  void loadMobGraphics() {
    for (auto i = 0; i < MOB_COUNT; i++) {
      mobSprites[i] = new olc::Sprite("./Baddies/" + mobFiles[i]);
      mobDecals[i] = new olc::Decal(mobSprites[i]);
    }
  }

  int map[MAP_WIDTH][MAP_HEIGHT][2];

  void loadTileGraphics() {
    for (auto i = 0; i < TILE_COUNT; i++) {
      tileSprites[i] = new olc::Sprite("./Tiles/" + tileFiles[i]);
      tileDecals[i] = new olc::Decal(tileSprites[i]);
    }
  }

  void resetMap() {
    for (auto x = 0; x < MAP_WIDTH; x++) {
      for (auto y = 0; y < MAP_HEIGHT; y++) {
        map[x][y][0] = 10;  // Mud.png
        map[x][y][1] = 0;
      }
    }
  }

  void saveMap() {
    auto mapFile = std::ofstream("map.csv");
    if (!mapFile.is_open()) return;
    for (auto layer = 0; layer < 2; layer++) {
      for (auto row = 0; row < MAP_HEIGHT; row++) {
        for (auto col = 0; col < MAP_WIDTH; col++) {
          mapFile << std::to_string(map[col][row][layer]);
          if (col != MAP_WIDTH - 1) {
            mapFile << ",";
          } else {
            mapFile << std::endl;
          }
        }
      }
    }
    mapFile.close();
  }

  void loadMap() {
    auto mapFile = std::ifstream("map.csv");
    if (!mapFile.is_open()) return;
    std::string line;
    std::string token;
    for (auto layer = 0; layer < 2; layer++) {
      for (auto row = 0; row < MAP_HEIGHT; row++) {
        std::getline(mapFile, line, '\n');
        auto stream = std::istringstream(line);
        for (auto col = 0; col < MAP_WIDTH; col++) {
          std::getline(stream, token, ',');
          if (token == "") continue;
          map[col][row][layer] = std::stoi(token);
        }
      }
    }
    mapFile.close();
  }

 public:
  bool OnUserCreate() override {
    /*
      Load resources here
    */
    loadTileGraphics();
    loadMobGraphics();
    resetMap();
    loadMap();
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
      map[x][y][0] = currentTile;
    } else if (GetMouse(1).bHeld) {
      currentTile = map[x][y][0];
    }

    // Start and end points for enemies (known as A and B)
    if (GetKey(olc::Key::A).bPressed) map[x][y][1] = 'A';
    if (GetKey(olc::Key::B).bPressed) map[x][y][1] = 'B';

    // Direction changes for enemies, north, south, east and west
    if (GetKey(olc::Key::N).bPressed) map[x][y][1] = 'N';
    if (GetKey(olc::Key::S).bPressed) map[x][y][1] = 'S';
    if (GetKey(olc::Key::E).bPressed) map[x][y][1] = 'E';
    if (GetKey(olc::Key::W).bPressed) map[x][y][1] = 'W';

    // Delete the current label
    if (GetKey(olc::Key::DEL).bPressed) map[x][y][1] = 0;

    if (GetKey(olc::Key::K1).bPressed) mobs.push_back(Mob(0, x, y));
    if (GetKey(olc::Key::K2).bPressed) mobs.push_back(Mob(1, x, y));
    if (GetKey(olc::Key::K3).bPressed) mobs.push_back(Mob(2, x, y));
    if (GetKey(olc::Key::K4).bPressed) mobs.push_back(Mob(3, x, y));
    if (GetKey(olc::Key::K5).bPressed) mobs.push_back(Mob(4, x, y));
    if (GetKey(olc::Key::K6).bPressed) mobs.push_back(Mob(5, x, y));
  }

  void processes() {
    /*
      Game logic goes here
    */

    for (auto i = 0; i < mobs.size();) {
      mobs[i].frame += GetElapsedTime() * 10;

      auto frameCount = mobSprites[mobs[i].type]->width / MOB_SIZE;
      if (mobs[i].frame >= frameCount) {
        mobs[i].frame -= frameCount;
      }

      mobs[i].progress += GetElapsedTime() * mobs[i].speed;

      if (mobs[i].progress > 1) {
        mobs[i].progress = 0;
        mobs[i].x = mobs[i].nextX;
        mobs[i].y = mobs[i].nextY;
        if (map[mobs[i].x][mobs[i].y][1] != 0) {
          mobs[i].direction = map[mobs[i].x][mobs[i].y][1];
        }

        if (mobs[i].direction == 'N') mobs[i].nextY--;
        if (mobs[i].direction == 'S') mobs[i].nextY++;
        if (mobs[i].direction == 'W') mobs[i].nextX--;
        if (mobs[i].direction == 'E') mobs[i].nextX++;
      }

      if (mobs[i].direction == 'B' || mobs[i].x < 0 || mobs[i].y < 0 ||
          mobs[i].x > MAP_WIDTH || mobs[i].y > MAP_HEIGHT) {
        mobs.erase(mobs.begin() + i);
      } else {
        i++;
      }
    }
  }

  void outputs() {
    SetPixelMode(olc::Pixel::NORMAL);

    /*
      Game graphics drawn here
    */
    for (auto x = 0; x < MAP_WIDTH; x++) {
      for (auto y = 0; y < MAP_HEIGHT; y++) {
        auto index = map[x][y][0];
        auto position = olc::vi2d{x * TILE_SIZE, y * TILE_SIZE};
        DrawDecal(position, tileDecals[index]);
        auto code = map[x][y][1];
        if (code != 0) {
          auto label = std::string(1, code);
          DrawStringDecal(position + olc::vi2d{13, 13}, label);
        }
      }
    }

    for (auto mob : mobs) {
      auto x = mob.x + mob.progress * (mob.nextX - mob.x);
      auto y = mob.y + mob.progress * (mob.nextY - mob.y);
      auto position = olc::vf2d{x * TILE_SIZE, y * TILE_SIZE};
      auto decal = mobDecals[mob.type];
      auto offset = olc::vi2d{MOB_SIZE * int(mob.frame), 0};
      auto size = olc::vi2d{MOB_SIZE, MOB_SIZE};
      DrawPartialDecal(position, size, decal, offset, size);
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
    for (auto i = 0; i < MOB_COUNT; i++) {
      delete mobSprites[i];
      delete mobDecals[i];
    }
    saveMap();
    return true;
  }
};

int main() {
  Game game;
  if (game.Construct(WINDOW_WIDTH, WINDOW_HEIGHT, 1, 1)) game.Start();
  return 0;
}
