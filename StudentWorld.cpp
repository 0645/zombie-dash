#include "StudentWorld.h"
#include "GameConstants.h"
#include "Level.h"
#include <string>
using namespace std;

GameWorld* createStudentWorld(string assetPath)
{
	return new StudentWorld(assetPath);
}

StudentWorld::StudentWorld(string assetPath)
: GameWorld(assetPath)
{
    player = nullptr;
}

StudentWorld::~StudentWorld()
{
    cleanUp();
}

int StudentWorld::init()
{
    Level lev(assetPath());
    char levelFile[12];
    sprintf(levelFile, "level%02d.txt", getLevel());
    Level::LoadResult result = lev.loadLevel(levelFile);

    if (result == Level::load_fail_file_not_found)
        return GWSTATUS_PLAYER_WON;
    else if (result == Level::load_fail_bad_format)
        return GWSTATUS_LEVEL_ERROR;
    else if (result == Level::load_success)
    {
        for(int y = 0; y < LEVEL_HEIGHT; y++)
        {
            for(int x = 0; x < LEVEL_WIDTH; x++)
            {
                Level::MazeEntry me = lev.getContentsOf(x, y);
                int startX = SPRITE_WIDTH * x;
                int startY = SPRITE_HEIGHT * y;
                switch(me)
                {
                    case Level::empty: break;
                    case Level::exit: actors.push_back(new Exit(startX, startY, this)); break;
                    case Level::player: player = new Penelope(startX, startY, this); break;
                    case Level::dumb_zombie: actors.push_back(new DumbZombie(startX, startY, this)); break;
                    case Level::smart_zombie: actors.push_back(new SmartZombie(startX, startY, this)); break;
                    case Level::citizen: actors.push_back(new Citizen(startX, startY, this)); break;
                    case Level::wall: actors.push_back(new Wall(startX, startY, this)); break;
                    case Level::pit: actors.push_back(new Pit(startX, startY, this)); break;
                    case Level::vaccine_goodie: actors.push_back(new VaccineGoodie(startX, startY, this)); break;
                    case Level::gas_can_goodie: actors.push_back(new GasCanGoodie(startX, startY, this)); break;
                    case Level::landmine_goodie: actors.push_back(new LandmineGoodie(startX, startY, this)); break;
                }
            }
        }
        return GWSTATUS_CONTINUE_GAME;
    }
    return -1;
}

int StudentWorld::move()
{
    player->doSomething();
    if(!player->isAlive())
    {
        decLives();
        return GWSTATUS_PLAYER_DIED;
    }

    int citizens = 0;
    for(auto i = actors.begin(); i != actors.end(); i++)
    {
        auto actor = *i;
        actor->doSomething();
        if(actor->isPerson())
            citizens++;
        if(!player->isAlive())
            {
                decLives();
                return GWSTATUS_PLAYER_DIED;
            }
    }

    if(!citizens) {
        for(auto i = actors.begin(); i != actors.end(); i++)
        {
            auto actor = *i;
            bool isExit = (!actor->blocksMovement() && actor->blocksFlames());
            if(isExit && actor->overlaps(player))
            {
                playSound(SOUND_LEVEL_FINISHED);
                return GWSTATUS_FINISHED_LEVEL;
            }
        }
    }
    
    auto i = actors.begin();
    while(i != actors.end())
    {
        if((*i)->isAlive())
            i++;
        else
        {
            delete (*i);
            i = actors.erase(i);
        }
    }
    
    char gameStatText[256];
    sprintf(
        gameStatText,
        "Score: %06d  Level: %d  Lives: %d  Vaccines: %d  Flames: %d  Mines: %d  Infected: %d",
        getScore(),
        getLevel(),
        getLives(),
        getPlayer()->vaccines(),
        getPlayer()->flames(),
        getPlayer()->mines(),
        getPlayer()->getInfectionCount()
    );
    setGameStatText(gameStatText);

    return GWSTATUS_CONTINUE_GAME;
}

void StudentWorld::cleanUp()
{
    delete player;
    player = nullptr;

    for(auto i = actors.begin(); i != actors.end(); i++)
    {
        delete (*i);
        (*i) = nullptr;
    }
    actors.clear();
}

Penelope * StudentWorld::getPlayer() const { return player; }
std::list<Actor *> StudentWorld::getActors() const { return actors; }

void StudentWorld::spawnDumbZombie(int startX, int startY) { actors.push_back(new DumbZombie(startX, startY, this)); }
void StudentWorld::spawnSmartZombie(int startX, int startY) { actors.push_back(new SmartZombie(startX, startY, this)); }
void StudentWorld::spawnVaccineGoodie(int startX, int startY) { actors.push_back(new VaccineGoodie(startX, startY, this)); }
void StudentWorld::spawnFlame(int startX, int startY, int startDirection) { actors.push_back(new Flame(startX, startY, startDirection, this)); }
void StudentWorld::spawnVomit(int startX, int startY, int startDirection) { actors.push_back(new Vomit(startX, startY, startDirection, this)); }
void StudentWorld::spawnLandmine(int startX, int startY) { actors.push_back(new Landmine(startX, startY, this)); }
void StudentWorld::spawnPit(int startX, int startY) { actors.push_back(new Pit(startX, startY, this)); }

void StudentWorld::givePlayerVaccines() { player->giveVaccines(1); }
void StudentWorld::givePlayerCharges() { player->giveCharges(5); }
void StudentWorld::givePlayerLandmines() { player->giveLandmines(2); }

bool StudentWorld::anyMovementBlockingActorsIntersectingExcluding(int dest_x, int dest_y, const Actor * a) const
{
    if(player->blocksMovement() && player != a)
    {
        int dx = abs(dest_x - player->getX());
        int dy = abs(dest_y - player->getY());
        if(dx < SPRITE_WIDTH && dy < SPRITE_HEIGHT)
            return true;
    }

    for(auto i = actors.begin(); i != actors.end(); i++)
    {
        auto actor = *i;
        if(actor->blocksMovement() && actor != a)
        {
            int dx = abs(dest_x - actor->getX());
            int dy = abs(dest_y - actor->getY());
            if(dx < SPRITE_WIDTH && dy < SPRITE_HEIGHT)
                return true;
        }
    }
    return false;
}

bool StudentWorld::anyOverlappingActorsAt(int x, int y, bool blocksFlames, bool person) const
{
    for(auto i = actors.begin(); i != actors.end(); i++)
    {
        auto actor = *i;
        bool bff = blocksFlames ? actor->blocksFlames() : 1;
        bool ipf = person ? actor->isPerson() : 1;
        if(actor->overlaps(x, y) && bff && ipf)
            return true;
    }
    return false;
}
bool StudentWorld::anyFlameBlockingActorsAt(int dest_x, int dest_y) const { return anyOverlappingActorsAt(dest_x, dest_y, true, false); }
bool StudentWorld::anyOverlappingActorsAt(int x, int y) const { return anyOverlappingActorsAt(x, y, false, false); }
bool StudentWorld::anyOverlappingCitizensAt(int x, int y) const { return anyOverlappingActorsAt(x, y, false, true); }
bool StudentWorld::playerOverlaps(int x, int y) const { return player->overlaps(x, y); }

void StudentWorld::doSomethingToActorsAt(int x, int y, std::string action, Actor * landmine, bool isPerson)
{
    for(auto i = actors.begin(); i != actors.end(); i++)
    {
        auto actor = *i;
        bool lmf = landmine ? actor->triggersLandmine() : 1;
        bool ipf = isPerson ? actor->isPerson() : 1;
        if(actor->overlaps(x, y) && lmf && ipf)
        {
            if(landmine)
                landmine->damage();
            else if(action == "damage")
                actor->damage();
            else if(action == "infect")
                actor->infect();
            else if(action == "save")
            {
                playSound(SOUND_CITIZEN_SAVED);
                actor->rewardPlayer();
                actor->setDead();
            }
        }
    }
}
void StudentWorld::damageActorsAt(int x, int y) { doSomethingToActorsAt(x, y, "damage", nullptr, false); }
void StudentWorld::infectActorsAt(int x, int y) { doSomethingToActorsAt(x, y, "infect", nullptr, false); }
void StudentWorld::damageLandmineTriggeringActorsAt(int x, int y, Actor * landmine) { doSomethingToActorsAt(x, y, "damage", landmine, false); }
void StudentWorld::saveCitizensAt(int x, int y) { doSomethingToActorsAt(x, y, "save", nullptr, true); }
