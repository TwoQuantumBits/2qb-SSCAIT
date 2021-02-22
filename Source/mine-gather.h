#pragma once

using namespace BWAPI;
using namespace Filter;

#include <unordered_map>

class MineGather {
private:
	std::unordered_map<int, int> distribution;
	std::unordered_map<int, int> worker_count;
public:
	void DoneGathering(Unit unit) {
		auto found = distribution.find(unit->getID());
		if (found == distribution.end()) return;
		worker_count[found->second]--;
		distribution.erase(found);
	}
	void AssignMine(std::unordered_set<Unit> wk, std::unordered_set<Unit> mn) {
        for (Unit unit : wk) {
            if (distribution.find(unit->getID()) != distribution.end()) continue;
            int min_worker = -1;
            for (Unit mine : mn) {
                int count = worker_count[mine->getID()];
                if (min_worker == -1 || count < min_worker) min_worker = count;
            }
            int choose;
            int min_dist = -1;
            for (Unit mine : mn) {
                if (worker_count[mine->getID()] != min_worker) continue;
                int dist = unit->getDistance(mine->getPosition());
                if (min_dist == -1 || dist < min_dist) {
                    min_dist = dist;
                    choose = mine->getID();
                }
            }
            worker_count[choose]++;
            distribution[unit->getID()] = choose;
        }
	}
	void Gather(Unit unit) {
		auto found = distribution.find(unit->getID());
		if (found != distribution.end()) unit->gather(Broodwar->getUnit(found->second));
	}
};