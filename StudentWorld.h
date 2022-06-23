#ifndef STUDENTWORLD_H_
#define STUDENTWORLD_H_

#include "GameWorld.h"
#include "Actor.h"
#include <string>
#include <list>

class Actor;
class Penelope;

class StudentWorld : public GameWorld
{
public:
    StudentWorld(std::string assetPath);
    ~StudentWorld();
    virtual int init();
    virtual int move();
    virtual void cleanUp();
    Penelope * getPlayer() const;
    std::list<Actor *> getActors() const;
    bool anyMovementBlockingActorsIntersectingExcluding(int dest_x, int dest_y, const Actor * a) const;
    bool anyFlameBlockingActorsAt(int dest_x, int dest_y) const;
    bool anyOverlappingActorsAt(int x, int y) const;
    bool anyOverlappingCitizensAt(int x, int y) const;
    bool playerOverlaps(int x, int y) const;
    void spawnFlame(int startX, int startY, int startDirection);
    void spawnPit(int startX, int startY);
    void spawnLandmine(int startX, int startY);
    void spawnVomit(int startX, int startY, int startDirection);
    void spawnVaccineGoodie(int startX, int startY);
    void spawnDumbZombie(int startX, int startY);
    void spawnSmartZombie(int startX, int startY);
    void givePlayerVaccines();
    void givePlayerCharges();
    void givePlayerLandmines();
    void saveCitizensAt(int x, int y);
    void damageActorsAt(int x, int y);
    void infectActorsAt(int x, int y);
    void damageLandmineTriggeringActorsAt(int x, int y, Actor * landmine);
private:
    Penelope * player;
    std::list<Actor *> actors;
    bool anyOverlappingActorsAt(int x, int y, bool blocksFlames, bool person) const;
    void doSomethingToActorsAt(int x, int y, std::string action, Actor * landmine, bool isPerson);
};

#endif // STUDENTWORLD_H_
