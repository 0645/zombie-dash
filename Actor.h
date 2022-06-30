#ifndef ACTOR_H_
#define ACTOR_H_

#include "GraphObject.h"
#include "StudentWorld.h"

class StudentWorld;

class Actor: public GraphObject
{
public:
    Actor(int imageID, double startX, double startY, int startDirection, int depth, StudentWorld * world);
    virtual bool blocksFlames() const;
    virtual bool blocksMovement() const;
    virtual bool isPerson() const;
    virtual bool isZombie() const;
    virtual bool triggersLandmine() const;
    virtual void damage();
    virtual void infect();
    virtual void rewardPlayer();
    virtual void doSomething() = 0;
    bool overlaps(double x, double y) const;
    bool overlaps(Actor * actor) const;
    bool isAlive() const;
    void setDead();
protected:
    StudentWorld * getWorld() const;
private:
    bool alive;
    StudentWorld * world;
};

class Agent: public Actor
{
public:
    Agent(int imageID, double startX, double startY, StudentWorld * world);
    virtual bool blocksMovement() const;
    virtual bool triggersLandmine() const;
protected:
    bool isParalized();
    void moveForward(int n);
    int getHorizontalFacingDirection(Actor * actor) const;
    int getVerticalFacingDirection(Actor * actor) const;
    double getResultX(int magnitude, int direction) const;
    double getResultY(int magnitude, int direction) const;
    bool canMoveTo(double dest_x, double dest_y) const;
private:
    bool paralized;
};

class Person: public Agent
{
public:
    Person(int imageID, double startX, double startY, StudentWorld * world);
    virtual bool isPerson() const;
    virtual void infect();
    int getInfectionCount() const;
protected:
    void disinfect();
    bool isFullyInfected();
    virtual void becomeZombie() = 0;
private:
    bool infected;
    int infectionCount;
};

class Penelope: public Person
{
public:
    Penelope(double startX, double start, StudentWorld * world);
    virtual void damage();
    virtual void doSomething();
    int vaccines() const;
    int flames() const;
    int mines() const;
    void giveVaccines(int n);
    void giveCharges(int n);
    void giveLandmines(int n);
private:
    int nVaccines;
    int nCharges;
    int nLandmines;
    virtual void becomeZombie();
    void move(int direction);
    void useVaccine();
    void useCharge();
    void useLandmine();
};

class Citizen: public Person
{
public:
    Citizen(double startX, double startY, StudentWorld * world);
    virtual void damage();
    virtual void infect();
    virtual void rewardPlayer();
    virtual void doSomething();
private:
    virtual void becomeZombie();
};

class Zombie: public Agent
{
public:
    Zombie(double startX, double startY, StudentWorld * world);
    virtual bool isZombie() const;
    virtual void damage();
    virtual void doSomething();
protected:
    int getRandomDirection() const;
private:
    int movementPlan;
    virtual void pickDirection() = 0;
    bool vomit();
};

class DumbZombie: public Zombie
{
public:
    DumbZombie(double startX, double startY, StudentWorld * world);
    virtual void rewardPlayer();
private:
    virtual void pickDirection();
    void dropVaccine();
};

class SmartZombie: public Zombie
{
public:
    SmartZombie(double startX, double startY, StudentWorld * world);
    virtual void rewardPlayer();
private:
    virtual void pickDirection();
};

class Goodie: public Actor
{
public:
    Goodie(int imageID, double startX, double startY, StudentWorld * world);
    virtual void damage();
    virtual void rewardPlayer();
    virtual void doSomething();
private:
    virtual void giveToPlayer() = 0;
};

class VaccineGoodie: public Goodie
{
public:
    VaccineGoodie(double startX, double startY, StudentWorld * world);
private:
    virtual void giveToPlayer();
};

class GasCanGoodie: public Goodie
{
public:
    GasCanGoodie(double startX, double startY, StudentWorld * world);
private:
    virtual void giveToPlayer();
};

class LandmineGoodie: public Goodie
{
public:
    LandmineGoodie(double startX, double startY, StudentWorld * world);
private:
    virtual void giveToPlayer();
};

class Projectile: public Actor
{
public:
    Projectile(int imageID, double startX, double startY, int startDirection, StudentWorld * world);
    virtual void doSomething();
private:
    int timeToLive;
    virtual void harm(Actor * actor) = 0;
};

class Flame: public Projectile
{
public:
    Flame(double startX, double startY, int startDirection, StudentWorld * world);
    virtual bool triggersLandmine() const;
private:
    virtual void harm(Actor * actor);
};

class Vomit: public Projectile
{
public:
    Vomit(double startX, double startY, int startDirection, StudentWorld * world);
private:
    virtual void harm(Actor * actor);
};

class Landmine: public Actor
{
public:
    Landmine(double startX, double startY, StudentWorld * world);
    virtual void damage();
    virtual void doSomething();
private:
    bool active;
    int nSafetyTicks;
};

class Wall: public Actor
{
public:
    Wall(double startX, double startY, StudentWorld * world);
    virtual bool blocksMovement() const;
    virtual bool blocksFlames() const;
    virtual void doSomething();
};

class Exit: public Actor
{
public:
    Exit(double startX, double startY, StudentWorld * world);
    virtual bool blocksFlames() const;
    virtual void doSomething();
};

class Pit: public Actor
{
public:
    Pit(double startX, double startY, StudentWorld * world);
    virtual void doSomething();
};

#endif // ACTOR_H_
