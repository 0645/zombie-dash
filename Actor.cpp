#include "Actor.h"
#include "GameWorld.h"

#include "GameConstants.h"

#include <cmath>

double distance(double x1, double y1, double x2, double y2)
{
    int delta_x = abs(x1 - x2);
    int delta_y = abs(y1 - y2);
    return sqrt(pow(delta_x, 2) + pow(delta_y, 2));
}

////////////////////////////////////////////////////////////////////////////////

Actor::Actor(int imageID, double startX, double startY, int startDirection, int depth, StudentWorld * world)
: GraphObject(imageID, startX, startY, startDirection, depth), alive(true), world(world) { }

bool Actor::blocksMovement() const { return false; }
bool Actor::blocksFlames() const { return false; }
bool Actor::isPerson() const { return false; }
bool Actor::isZombie() const { return false; }
bool Actor::triggersLandmine() const { return false; }

StudentWorld * Actor::getWorld() const { return world; }
bool Actor::isAlive() const { return alive; }

bool Actor::overlaps(double x, double y) const { return distance(getX(), getY(), x, y) <= 10; }

bool Actor::overlaps(Actor * actor) const { return overlaps(actor->getX(), actor->getY()); }

void Actor::setDead() { alive = false; }

void Actor::damage() { }

void Actor::infect() { }

void Actor::rewardPlayer() { }

////////////////////////////////////////////////////////////////////////////////

Person::Person(int imageID, double startX, double startY, StudentWorld * world)
: Agent(imageID, startX, startY, world), infected(false), infectionCount(0) { }

bool Person::isPerson() const { return true; }

int Person::getInfectionCount() const { return infectionCount; }

void Person::infect() { infected = true; }

void Person::disinfect()
{
    infected = false;
    infectionCount = 0;
}

bool Person::isFullyInfected()
{
    if(infected)
        infectionCount++;
    return infectionCount >= 500;
}

////////////////////////////////////////////////////////////////////////////////

Agent::Agent(int imageID, double startX, double startY, StudentWorld * world)
: Actor(imageID, startX, startY, right, 0, world), paralized(true) { }

bool Agent::blocksMovement() const { return true; }
bool Agent:: triggersLandmine() const { return true; }

bool Agent::isParalized()
{
    paralized = !paralized;
    return paralized;
}

int Agent::getHorizontalFacingDirection(Actor * actor) const
{
    if(getX() == actor->getX())
        return -1;

    double closestDistance = distance(getX(), getY(),
        actor->getX(), actor->getY());
    double d = distance(getX() - 1, getY(),
        actor->getX(), actor->getY());
    if(d < closestDistance)
        return left;
    else if(d > closestDistance)
        return right;
    else
        return -1;
}

int Agent::getVerticalFacingDirection(Actor * actor) const
{
    if(getY() == actor->getY())
        return -1;

    double closestDistance = distance(getX(), getY(),
        actor->getX(), actor->getY());
    double d = distance(getX(), getY() - 1,
        actor->getX(), actor->getY());
    if(d < closestDistance)
        return down;
    else if(d > closestDistance)
        return up;
    else
        return -1;
}

double Agent::getResultX(int magnitude, int direction) const
{
    if(direction == left)
        return getX() - magnitude;
    else if(direction == right)
        return getX() + magnitude;
    else
        return getX();
}

double Agent::getResultY(int magnitude, int direction) const
{
    if(direction == up)
        return getY() + magnitude;
    else if(direction == down)
        return getY() - magnitude;
    else
        return getY();
}

void Agent::moveForward(int n)
{
    int direction = getDirection();

    int delta_x = 0;
    int delta_y = 0;

    if(direction == left)
        delta_x -= n;
    else if(direction == right)
        delta_x += n;
    else if(direction == up)
        delta_y += n;
    else if(direction == down)
        delta_y -= n;

    int dest_x = getX() + delta_x;
    int dest_y = getY() + delta_y;

    if(canMoveTo(dest_x, dest_y))
        moveTo(dest_x, dest_y);
}

bool Agent::canMoveTo(double dest_x, double dest_y) const
{
    return !getWorld()->anyMovementBlockingActorsIntersectingExcluding(dest_x, dest_y, this);
}

////////////////////////////////////////////////////////////////////////////////

Penelope::Penelope(double startX, double startY, StudentWorld * world)
: Person(IID_PLAYER, startX, startY, world), nVaccines(0), nCharges(0), nLandmines(0) { }

int Penelope::vaccines() const { return nVaccines; }
int Penelope::flames() const { return nCharges; }
int Penelope::mines() const { return nLandmines; }

void Penelope::doSomething()
{
    if(!isAlive() || isFullyInfected())
    {
        becomeZombie();
        return;
    }
    int ch;
    if (getWorld()->getKey(ch))
    {
        switch (ch)
        {
            case KEY_PRESS_LEFT:
            case KEY_PRESS_RIGHT:
            case KEY_PRESS_UP:
            case KEY_PRESS_DOWN:
                move(ch);
                break;
            case KEY_PRESS_SPACE:
                useCharge();
                break;
            case KEY_PRESS_TAB:
                useLandmine();
                break;
            case KEY_PRESS_ENTER:
                useVaccine();
                break;
        }
    }
}

void Penelope::move(int direction) {
    if (direction == KEY_PRESS_LEFT)
        setDirection(left);
    else if (direction == KEY_PRESS_RIGHT)
        setDirection(right);
    else if (direction == KEY_PRESS_UP)
        setDirection(up);
    else if (direction == KEY_PRESS_DOWN)
        setDirection(down);
    
    moveForward(4);
}

void Penelope::giveVaccines(int n) { nVaccines += n; }
void Penelope::giveCharges(int n) { nCharges += n; }
void Penelope::giveLandmines(int n) { nLandmines += n; }

void Penelope::useVaccine()
{
    if(nVaccines > 0)
    {
        disinfect();
        nVaccines--;
    }
}

void Penelope::useCharge()
{
    int direction = getDirection();
    if(nCharges > 0)
    {
        for(int i = 1; i <= 3; i++)
        {
            int dest_x = getResultX(SPRITE_WIDTH * i, direction);
            int dest_y = getResultY(SPRITE_HEIGHT * i, direction);

            if(!getWorld()->anyFlameBlockingActorsAt(dest_x, dest_y))
                getWorld()->spawnFlame(dest_x, dest_y, direction);
            else
                break;
        }
        nCharges--;
    }
}

void Penelope::useLandmine()
{
    if(nLandmines > 0)
    {
        getWorld()->spawnLandmine(getX(), getY());
        nLandmines--;
    }
}

void Penelope::becomeZombie() { setDead(); }
void Penelope::damage() { setDead(); }

////////////////////////////////////////////////////////////////////////////////

Wall::Wall(double startX, double startY, StudentWorld * world)
: Actor(IID_WALL, startX, startY, right, 0, world) { }

bool Wall::blocksMovement() const { return true; }
bool Wall::blocksFlames() const { return true; }

void Wall::doSomething() { }

////////////////////////////////////////////////////////////////////////////////

Exit::Exit(double startX, double startY, StudentWorld * world)
: Actor(IID_EXIT, startX, startY, right, 1, world) { }

bool Exit::blocksFlames() const { return true; }

void Exit::doSomething() { getWorld()->saveCitizensAt(getX(), getY()); }

////////////////////////////////////////////////////////////////////////////////

Pit::Pit(double startX, double startY, StudentWorld * world)
: Actor(IID_PIT, startX, startY, right, 0, world) { }

void Pit::doSomething()
{
    auto player = getWorld()->getPlayer();
    if (overlaps(player))
        player->damage();

    getWorld()->damageActorsAt(getX(), getY());
}

////////////////////////////////////////////////////////////////////////////////

Goodie::Goodie(int imageID, double startX, double startY, StudentWorld * world)
: Actor(imageID, startX, startY, right, 1, world) { }

void Goodie::doSomething()
{
    if(!isAlive())
        return;
    if (getWorld()->playerOverlaps(getX(), getY()))
    {
        rewardPlayer();
        setDead();
    }
}

void Goodie::rewardPlayer()
{
    getWorld()->increaseScore(50);
    giveToPlayer();
}

void Goodie::damage() { setDead(); }

////////////////////////////////////////////////////////////////////////////////

VaccineGoodie::VaccineGoodie(double startX, double startY, StudentWorld * world)
: Goodie(IID_VACCINE_GOODIE, startX, startY, world) { }

void VaccineGoodie::giveToPlayer() { getWorld()->givePlayerVaccines(); }

////////////////////////////////////////////////////////////////////////////////

GasCanGoodie::GasCanGoodie(double startX, double startY, StudentWorld * world)
: Goodie(IID_GAS_CAN_GOODIE, startX, startY, world) { }

void GasCanGoodie::giveToPlayer() { getWorld()->givePlayerCharges(); }

////////////////////////////////////////////////////////////////////////////////

LandmineGoodie::LandmineGoodie(double startX, double startY, StudentWorld * world)
: Goodie(IID_LANDMINE_GOODIE, startX, startY, world) { }

void LandmineGoodie::giveToPlayer() { getWorld()->givePlayerLandmines(); }

////////////////////////////////////////////////////////////////////////////////

Projectile::Projectile(int imageID, double startX, double startY, int startDirection, StudentWorld * world)
: Actor(imageID, startX, startY, startDirection, 0, world), timeToLive(2) { }

void Projectile::doSomething()
{
    if(!isAlive())
        return;
    if(!timeToLive)
    {
        setDead();
        return;
    }

    auto player = getWorld()->getPlayer();
    if(overlaps(player))
        harm(player);
    
    auto actors = getWorld()->getActors();
    for(auto i = actors.begin(); i != actors.end(); i++)
    {
        auto actor = *i;
        if(overlaps(actor))
            harm(actor);
    }

    timeToLive--;
}

////////////////////////////////////////////////////////////////////////////////

Flame::Flame(double startX, double startY, int startDirection, StudentWorld * world)
: Projectile(IID_FLAME, startX, startY, startDirection, world) { }

bool Flame::triggersLandmine() const { return true; }

void Flame::harm(Actor * actor) { actor->damage(); }

////////////////////////////////////////////////////////////////////////////////

Vomit::Vomit(double startX, double startY, int startDirection, StudentWorld * world)
: Projectile(IID_VOMIT, startX, startY, startDirection, world) { }

void Vomit::harm(Actor * actor) { actor->infect(); }

////////////////////////////////////////////////////////////////////////////////

Landmine::Landmine(double startX, double startY, StudentWorld * world)
: Actor(IID_LANDMINE, startX, startY, right, 1, world), active(false), nSafetyTicks(30) { }

void Landmine::doSomething()
{
    if(!isAlive())
        return;
    else if(!active)
    {
        nSafetyTicks--;
        if(nSafetyTicks == 0)
            active = true;
    }
    else
    {
        if(getWorld()->playerOverlaps(getX(), getY()))
            damage();
        
        getWorld()->damageLandmineTriggeringActorsAt(getX(), getY(), this);
    }
}

void Landmine::damage()
{
    if(!isAlive())
        return;

    for(int i = -1; i <= 1; i++)
    {
        for(int j = -1; j <= 1; j++)
        {
            int dest_x = getX() + i * SPRITE_WIDTH;
            int dest_y = getY() + j * SPRITE_HEIGHT;

            if(!getWorld()->anyFlameBlockingActorsAt(dest_x, dest_y))
                getWorld()->spawnFlame(dest_x, dest_y, up);
        }
    }
    getWorld()->spawnPit(getX(), getY());

    setDead();
}

////////////////////////////////////////////////////////////////////////////////

Zombie::Zombie(double startX, double startY, StudentWorld * world)
: Agent(IID_ZOMBIE, startX, startY, world), movementPlan(0) { }

void Zombie::doSomething()
{
    if(!isAlive() || isParalized() || vomit())
        return;
    
    if(movementPlan == 0)
    {
        movementPlan = randInt(3, 10);
        pickDirection();
    }

    int direction = getDirection();
    int dest_x = getResultX(1, direction);
    int dest_y = getResultY(1, direction);

    if(!canMoveTo(dest_x, dest_y))
        movementPlan = 0;
    else
    {
        moveTo(dest_x, dest_y);
        movementPlan--;
    }
}

bool Zombie::vomit()
{
    int direction = getDirection();
    int vomit_x = getResultX(SPRITE_WIDTH, direction);
    int vomit_y = getResultY(SPRITE_HEIGHT, direction);

    if(randInt(1,3) == 1)
    {
        if(getWorld()->playerOverlaps(vomit_x, vomit_y))
        {
            getWorld()->spawnVomit(vomit_x, vomit_y, direction);
            return true;
        }
        
        if(getWorld()->anyOverlappingCitizensAt(vomit_x, vomit_y))
        {
            getWorld()->spawnVomit(vomit_x, vomit_y, direction);
            return true;
        }
    }

    return false;
}

void Zombie::damage()
{
    if(!isAlive())
        return;
    rewardPlayer();
    setDead();
}

bool Zombie::isZombie() const { return true; }

int Zombie::getRandomDirection() const { return randInt(0, 3) * 90; }

////////////////////////////////////////////////////////////////////////////////

DumbZombie::DumbZombie(double startX, double startY, StudentWorld * world)
: Zombie(startX, startY, world) { }

void DumbZombie::pickDirection()
{
    setDirection(getRandomDirection());
}

void DumbZombie::rewardPlayer()
{
    getWorld()->increaseScore(1000);
    if(randInt(1,10) == 1)
        dropVaccine();
}

void DumbZombie::dropVaccine()
{
    int direction = getRandomDirection();
    int vaccine_x = getResultX(SPRITE_WIDTH, direction);
    int vaccine_y = getResultY(SPRITE_HEIGHT, direction);

    if(!getWorld()->anyOverlappingActorsAt(vaccine_x, vaccine_y))
        getWorld()->spawnVaccineGoodie(vaccine_x, vaccine_y);
}

////////////////////////////////////////////////////////////////////////////////

SmartZombie::SmartZombie(double startX, double startY, StudentWorld * world)
: Zombie(startX, startY, world) { }

void SmartZombie::pickDirection()
{
    Actor * closestPerson = getWorld()->getPlayer();
    double closestDistance = distance(getX(), getY(),
        closestPerson->getX(), closestPerson->getY());

    auto actors = getWorld()->getActors();
    for(auto i = actors.begin(); i != actors.end(); i++)
    {
        auto actor = *i;
        double d = distance(getX(), getY(), actor->getX(), actor->getY());
        if(actor->isPerson() && d < closestDistance)
        {
            closestPerson = actor;
            closestDistance = d;
        }
    }

    if(closestDistance > 80)
        setDirection(getRandomDirection());
    else if(getX() == closestPerson->getX())
        setDirection(getVerticalFacingDirection(closestPerson));
    else if(getY() == closestPerson->getY())
        setDirection(getHorizontalFacingDirection(closestPerson));
    else if(randInt(1, 2) == 1)
        setDirection(getVerticalFacingDirection(closestPerson));
    else
        setDirection(getHorizontalFacingDirection(closestPerson));
}

void SmartZombie::rewardPlayer() { getWorld()->increaseScore(2000); }

////////////////////////////////////////////////////////////////////////////////

Citizen::Citizen(double startX, double startY, StudentWorld * world)
: Person(IID_CITIZEN, startX, startY, world) { }

void Citizen::damage()
{
    getWorld()->increaseScore(-1000);
    setDead();
}

void Citizen::becomeZombie()
{
    damage();
    if(randInt(1, 100) <= 70)
        getWorld()->spawnDumbZombie(getX(), getY());
    else
        getWorld()->spawnSmartZombie(getX(), getY());
}

void Citizen::doSomething()
{
    if(!isAlive())
        return;
    else if(isFullyInfected())
    {
        becomeZombie();
        return;
    }
    else if(isParalized())
        return;

    auto player = getWorld()->getPlayer();
    double dist_p = distance(getX(), getY(),
        player->getX(), player->getY());
    
    Actor * closestZombie = nullptr;
    double dist_z = dist_p + 1;
    auto actors = getWorld()->getActors();
    for(auto i = actors.begin(); i != actors.end(); i++)
    {
        auto actor = *i;
        double d = distance(getX(), getY(), actor->getX(), actor->getY());
        if(actor->isZombie() && d < dist_z)
        {
            closestZombie = actor;
            dist_z = d;
        }
    }

    if(dist_p < dist_z && dist_p <= 80)
    {
        int arr[] = {
            getHorizontalFacingDirection(player),
            getVerticalFacingDirection(player)
        };
        if(randInt(1, 2) == 1)
        {
            int temp = arr[0];
            arr[0] = arr[1];
            arr[1] = temp;
        }

        for(int i = 0; i < 2; i++)
        {
            int direction = arr[i];
            if(direction == -1)
                continue;
            
            int dest_x = getResultX(2, direction);
            int dest_y = getResultY(2, direction);
            if(
                !getWorld()->anyMovementBlockingActorsIntersectingExcluding(dest_x, dest_y, this) &&
                !getWorld()->anyMovementBlockingActorsIntersectingExcluding(dest_x, dest_y, player)
            )
                return;
            else if(canMoveTo(dest_x, dest_y))
            {
                setDirection(direction);
                moveTo(dest_x, dest_y);
                return;
            }
        }
    }

    if(closestZombie != nullptr && dist_z <= 80)
    {
        double furthestNearest = dist_z;
        int furthestNearestDirection = -1;

        for(int i = 0; i < 4; i++)
        {
            int direction = i * 90;
            int dest_x = getResultX(2, direction);
            int dest_y = getResultY(2, direction);
            if(canMoveTo(dest_x, dest_y))
            {
                double nearest = distance(dest_x, dest_y,
                    closestZombie->getX(), closestZombie->getY());
                for(auto i = actors.begin(); i != actors.end(); i++)
                {
                    auto actor = *i;
                    double d = distance(dest_x, dest_y, actor->getX(), actor->getY());
                    if(actor->isZombie() && d < nearest)
                        nearest = d;
                }
                if(nearest > furthestNearest)
                {
                    furthestNearest = nearest;
                    furthestNearestDirection = direction;
                }
            }
        }

        if(furthestNearestDirection != -1)
        {
            setDirection(furthestNearestDirection);
            moveForward(2);
        }
    }
}

void Citizen::rewardPlayer()
{
    getWorld()->increaseScore(500);
}
