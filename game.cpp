#define OLC_PGE_APPLICATION
#include "olcPixelGameEngine.h"

/* Game constants */

const int WINDOW_WIDTH = 1280;
const int WINDOW_HEIGHT = 720;

const int TILE_SIZE = 32;
const int TILE_COUNT = 26;

const int MAP_WIDTH = 40;
const int MAP_HEIGHT = 20;

const int MOB_SIZE = 32;
const int MOB_COUNT = 6;

const int TOWER_SIZE = 32;
const int TOWER_COUNT = 7;

const int PROJECTILE_SIZE = 16;
const int PROJECTILE_COUNT = 9;

/* Main game class */

class Game : public olc::PixelGameEngine {
 public:
  Game() { sAppName = "Shrek tower defence"; }

 private:
  float timer = 0;
  int frames = 0;
  int fps;

  int currentTile = 2;  // Grass1.png

  float spawnDelay = 2;
  float spawnTimer = spawnDelay;

  enum class MODE { EDIT, PLAY };

  MODE mode = MODE::PLAY;

  /* The graphics for all the tiles */

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

  /* The map that stores all the tiles and also the labels */

  int map[MAP_WIDTH][MAP_HEIGHT][2];

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

  /* The graphics for all the mobs */

  olc::Sprite* mobSprites[MOB_COUNT];
  olc::Decal* mobDecals[MOB_COUNT];

  const std::string mobFiles[MOB_COUNT]{
      "DoTheRoarKid.png", "FairyGodmother.png", "knight.png",
      "lordfarquaad.png", "Pied piper.png",     "Pied piper_s mice.png",
  };

  /* Mob 'struct' - just like a class but all attributes
     and methods are automatically public. */

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

  /* Vector (dynamic list) of mobs */

  std::vector<Mob> mobs;

  /* The graphics for towers */

  olc::Sprite* towerSprites[TOWER_COUNT];
  olc::Decal* towerDecals[TOWER_COUNT];

  const std::string towerFiles[TOWER_COUNT]{
      "donkey.png",        "Dragon.png",          "gingerbreadSpriteSheet.png",
      "Puss_in_boots.png", "rumpelstiltskin.png", "shrek.png",
      "smashMouth.png",
  };

  struct Tower {
    int type;
    int x;
    int y;
    float frame;
    float reloadTimer;
    float reloadDelay;
    int projectileType;
    Tower(int givenType, int givenX, int givenY, int givenDelay,
          int givenProjectile) {
      type = givenType;
      x = givenX;
      y = givenY;
      frame = 0;
      reloadTimer = givenDelay;
      reloadDelay = givenDelay;
      projectileType = givenProjectile;
    }
  };

  /* Vector (dynamic list) of mobs */

  std::vector<Tower> towers;

  /* The graphics for projectiles */

  olc::Sprite* projectileSprites[PROJECTILE_COUNT];
  olc::Decal* projectileDecals[PROJECTILE_COUNT];

  const std::string projectileFiles[PROJECTILE_COUNT]{
      "Boot_throw.png", "donkeydragon.png", "fireball.png",
      "Javelin.png",    "lollipop.png",     "magic.png",
      "mirror.png",     "onion.png",        "smashmMouthSpit.png",
  };

  struct Projectile {
    int type;
    olc::vf2d position;
    olc::vf2d velocity;
    float frame;
    Projectile(int givenType, olc::vf2d givenPosition,
               olc::vf2d givenVelocity) {
      type = givenType;
      position = givenPosition;
      velocity = givenVelocity;
      frame = 0;
    }
  };

  std::vector<Projectile> projectiles;

 public:
  bool OnUserCreate() override {
    /*
      Load resources here
    */
    loadGraphics(TILE_COUNT, tileSprites, tileDecals, tileFiles, "./Tiles/");
    loadGraphics(MOB_COUNT, mobSprites, mobDecals, mobFiles, "./Baddies/");
    loadGraphics(TOWER_COUNT, towerSprites, towerDecals, towerFiles,
                 "./Towers/");
    loadGraphics(PROJECTILE_COUNT, projectileSprites, projectileDecals,
                 projectileFiles, "./Projectiles/");
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

  /* Functions for loading and deleting graphics (resource management)
   */

  void loadGraphics(int count, olc::Sprite* sprites[], olc::Decal* decals[],
                    const std::string files[], std::string path) {
    for (auto i = 0; i < count; i++) {
      sprites[i] = new olc::Sprite(path + files[i]);
      decals[i] = new olc::Decal(sprites[i]);
    }
  }

  void deleteGraphics(int count, olc::Sprite* sprites[], olc::Decal* decals[]) {
    for (auto i = 0; i < count; i++) {
      delete sprites[i];
      delete decals[i];
    }
  }

  void inputs() {
    /*
      Game controls goes here
    */

    if (GetKey(olc::Key::TAB).bPressed) {
      mode = (mode == MODE::EDIT) ? MODE::PLAY : MODE::EDIT;
    }

    if (mode == MODE::EDIT) {
      /* Up and down arrow change currently selected tile */
      if (GetKey(olc::Key::DOWN).bPressed) {
        currentTile++;
        if (currentTile > TILE_COUNT - 1) currentTile = 0;
      }

      if (GetKey(olc::Key::UP).bPressed) {
        currentTile--;
        if (currentTile < 0) currentTile = TILE_COUNT - 1;
      }

      /* x and y store the location in tile coordinates of the mouse cursor*/

      auto x = GetMouseX() / TILE_SIZE;
      auto y = GetMouseY() / TILE_SIZE;

      if (x < 0) x = 0;
      if (x > MAP_WIDTH - 1) x = MAP_WIDTH - 1;
      if (y < 0) y = 0;
      if (y > MAP_HEIGHT - 1) y = MAP_HEIGHT - 1;

      /* Left mouse button (0) and right mouse button (1) */
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
  }

  void processes() {
    /*
      Game logic goes here
    */

    if (mode == MODE::PLAY) {
      /* Spawn random mobs */

      spawnTimer -= GetElapsedTime();

      if (spawnTimer <= 0) {
        std::vector<olc::vi2d> spawnPoints;
        for (auto x = 0; x < MAP_WIDTH; x++) {
          for (auto y = 0; y < MAP_HEIGHT; y++) {
            if (map[x][y][1] == 'A') {
              spawnPoints.push_back(olc::vi2d(x, y));
            }
          }
        }

        if (spawnPoints.size() > 0) {
          auto t = rand() % MOB_COUNT;
          auto p = rand() % spawnPoints.size();
          auto x = spawnPoints[p].x;
          auto y = spawnPoints[p].y;
          mobs.push_back(Mob(t, x, y));
        }

        spawnTimer = spawnDelay;
      }

      /* Update all the mobs...*/
      for (auto i = 0; i < mobs.size();) {
        /* First, their animation frames */

        mobs[i].frame += GetElapsedTime() * 10;

        auto frameCount = mobSprites[mobs[i].type]->width / MOB_SIZE;
        if (mobs[i].frame >= frameCount) {
          mobs[i].frame -= frameCount;
        }

        /* Then, their progress (lerping - linear interpolation) */

        mobs[i].progress += GetElapsedTime() * mobs[i].speed;

        /* If they reach the detination location, work out the next detination
         */

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

        /* Delete any mobs that reach the 'B' label or have moved off the map */

        if (mobs[i].direction == 'B' || mobs[i].x < 0 || mobs[i].y < 0 ||
            mobs[i].x > MAP_WIDTH || mobs[i].y > MAP_HEIGHT) {
          mobs.erase(mobs.begin() + i);
        } else {
          i++;
        }
      }
    }
  }

  void outputs() {
    SetPixelMode(olc::Pixel::NORMAL);

    /*
      Game graphics drawn here
    */

    /* First, draw all the tiles */

    for (auto x = 0; x < MAP_WIDTH; x++) {
      for (auto y = 0; y < MAP_HEIGHT; y++) {
        auto index = map[x][y][0];
        auto position = olc::vi2d{x * TILE_SIZE, y * TILE_SIZE};
        DrawDecal(position, tileDecals[index]);

        /* Add a label if needed */
        if (mode == MODE::EDIT) {
          auto code = map[x][y][1];
          if (code != 0) {
            auto label = std::string(1, code);
            DrawStringDecal(position + olc::vi2d{13, 13}, label);
          }
        }
      }
    }

    /* Then, draw all the mobs */

    for (auto mob : mobs) {
      auto x = mob.x + mob.progress * (mob.nextX - mob.x);
      auto y = mob.y + mob.progress * (mob.nextY - mob.y);
      auto position = olc::vf2d{x * TILE_SIZE, y * TILE_SIZE};
      auto decal = mobDecals[mob.type];
      auto offset = olc::vi2d{MOB_SIZE * int(mob.frame), 0};
      auto size = olc::vi2d{MOB_SIZE, MOB_SIZE};
      DrawPartialDecal(position, size, decal, offset, size);
    }

    /* Draw the currently selected tile in the bottom left */

    if (mode == MODE::EDIT) {
      auto position = olc::vi2d{20, WINDOW_HEIGHT - TILE_SIZE - 20};
      DrawDecal(position, tileDecals[currentTile]);
    }

    /* Add an FPS display */
    if (fps > 0) {
      auto fpsPosition = olc::vi2d(WINDOW_WIDTH - 70, WINDOW_HEIGHT - 70);
      DrawStringDecal(fpsPosition, "FPS " + std::to_string(fps));
    }
  }

  bool OnUserDestroy() override {
    std::cout << "Closing game" << std::endl;
    saveMap();
    deleteGraphics(TILE_COUNT, tileSprites, tileDecals);
    deleteGraphics(MOB_COUNT, mobSprites, mobDecals);
    deleteGraphics(TOWER_COUNT, towerSprites, towerDecals);
    deleteGraphics(PROJECTILE_COUNT, projectileSprites, projectileDecals);
    return true;
  }
};

int main() {
  Game game;
  if (game.Construct(WINDOW_WIDTH, WINDOW_HEIGHT, 1, 1)) game.Start();
  return 0;
}
