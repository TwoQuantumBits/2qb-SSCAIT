#pragma once

using namespace BWAPI;
using namespace Filter;

class UnitContainer {
public:

    std::unordered_set<Unit> unit_set;
    void OnFrame() {
        for (Unit u : unit_set) {
            // Ignore the unit if it no longer exists
            // Make sure to include this block when handling any Unit pointer!
            if (!u->exists())
                continue;

            // Ignore the unit if it has one of the following status ailments
            if (u->isLockedDown() || u->isMaelstrommed() || u->isStasised())
                continue;

            // Ignore the unit if it is in one of the following states
            if (u->isLoaded() || !u->isPowered() || u->isStuck())
                continue;

            // Ignore the unit if it is incomplete or busy constructing
            if (!u->isCompleted() || u->isConstructing())
                continue;

            PerformTask(u);
        }
    }
    void Insert(Unit unit) {
        if (CheckType(unit)) unit_set.insert(unit);
    }
    void Remove(Unit unit) {
        auto found = unit_set.find(unit);
        if (found != unit_set.end()) unit_set.erase(found);
    }
protected:
    virtual bool CheckType(Unit unit) = 0;
    virtual void PerformTask(Unit u) = 0;
};

class ResourceDepotContainer : public UnitContainer {
public:
    bool CheckType(Unit unit) { return (unit->getType().isResourceDepot() && unit->getPlayer() == Broodwar->self()); }
private:
    void PerformTask(Unit u) {
        // Order the depot to construct more workers! But only when it is idle.
        if (u->isIdle() && !u->train(u->getType().getRace().getWorker()))
        {
            // If that fails, draw the error at the location so that you can visibly see what went wrong!
            // However, drawing the error once will only appear for a single frame
            // so create an event that keeps it on the screen for some frames
            Position pos = u->getPosition();
            Error lastErr = Broodwar->getLastError();
            Broodwar->registerEvent([pos, lastErr](Game*) { Broodwar->drawTextMap(pos, "%c%s", Text::White, lastErr.c_str()); },   // action
                nullptr,    // condition
                Broodwar->getLatencyFrames());  // frames to run

// Retrieve the supply provider type in the case that we have run out of supplies
            UnitType supplyProviderType = u->getType().getRace().getSupplyProvider();
            static int lastChecked = 0;

            // If we are supply blocked and haven't tried constructing more recently
            if (lastErr == Errors::Insufficient_Supply &&
                lastChecked + 400 < Broodwar->getFrameCount() &&
                Broodwar->self()->incompleteUnitCount(supplyProviderType) == 0)
            {
                lastChecked = Broodwar->getFrameCount();

                // Retrieve a unit that is capable of constructing the supply needed
                Unit supplyBuilder = u->getClosestUnit(GetType == supplyProviderType.whatBuilds().first &&
                    (IsIdle || IsGatheringMinerals) &&
                    IsOwned);
                // If a unit was found
                if (supplyBuilder)
                {
                    if (supplyProviderType.isBuilding())
                    {
                        TilePosition targetBuildLocation = Broodwar->getBuildLocation(supplyProviderType, supplyBuilder->getTilePosition());
                        if (targetBuildLocation)
                        {
                            // Register an event that draws the target build location
                            Broodwar->registerEvent([targetBuildLocation, supplyProviderType](Game*)
                                {
                                    Broodwar->drawBoxMap(Position(targetBuildLocation),
                                        Position(targetBuildLocation + supplyProviderType.tileSize()),
                                        Colors::Blue);
                                },
                                nullptr,  // condition
                                    supplyProviderType.buildTime() + 100);  // frames to run

            // Order the builder to construct the supply structure
                            supplyBuilder->build(supplyProviderType, targetBuildLocation);
                        }
                    }
                    else
                    {
                        // Train the supply provider (Overlord) if the provider is not a structure
                        supplyBuilder->train(supplyProviderType);
                    }
                } // closure: supplyBuilder is valid
            } // closure: insufficient supply
        } // closure: failed to train idle unit
    }
};