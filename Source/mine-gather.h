#pragma once

using namespace BWAPI;
using namespace Filter;

#include <unordered_map>

class MineGather {
private:
	std::unordered_map<Unit, Unit> distribution;
	std::unordered_map<Unit, int> worker_count;
public:
	void DoneGathering(Unit unit) {
		auto found = distribution.find(unit);
		if (found == distribution.end()) return;
		worker_count[found->second]--;
		AssignMine(unit);
	}
	void Remove(Unit unit) {
		auto found = distribution.find(unit);
		if (found != distribution.end()) distribution.erase(found);
		auto found1 = worker_count.find(unit);
		if (found1 != worker_count.end()) worker_count.erase(found1);
	}
    void Insert(Unit unit) {
		if (unit->getType().isWorker() && unit->getPlayer() == Broodwar->self()) {
			AssignMine(unit);
		}
		else if (unit->getType().isMineralField() || unit->getType().isRefinery()) {
			worker_count.insert({ unit, 0 });
		}
    }
	void AssignMine(Unit unit) {
		auto found = distribution.find(unit);
		if (found != distribution.end()) distribution.erase(found);
		int min_worker = INT_MAX;
		int min_dist = INT_MAX;
		Unit choose = unit;
		for (auto i : worker_count) min_worker = std::min(min_worker, i.second);
		for (auto i : worker_count) {
			if (i.second > min_worker) continue;

			int dist = unit->getDistance(i.first);
			if (min_dist > dist) {
				min_dist = dist;
				choose = i.first;
			}
		}
		worker_count[choose]++;
		if (choose != unit) distribution.insert({ unit, choose });
	}
	void Gather(Unit u) {
		auto found = distribution.find(u);
		if (found != distribution.end()) u->gather(found->second);
		else AssignMine(u);
	}
	void OnFrame() {
		
		for (auto i : distribution) {
			Unit u = i.first;
			if (u->isIdle())
			{
				// Order workers carrying a resource to return them to the center,
				// otherwise find a mineral patch to harvest.
				if (u->isCarryingGas() || u->isCarryingMinerals())
				{
					u->returnCargo();
					DoneGathering(u);
				}
				else if (!u->getPowerUp())  // The worker cannot harvest anything if it
				{                             // is carrying a powerup such as a flag
					Gather(u);
				  // Harvest from the nearest mineral patch or gas refinery
					//if (!u->gather(distribution[u]))
					//{
					//	// If the call fails, then print the last error message
					//	Broodwar << "No fuck you, " << Broodwar->getLastError() << std::endl;
					//}

				} // closure: has no powerup
			} // closure: if idle
		}
	}
};