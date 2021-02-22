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
		distribution.erase(found);
	}
	void AssignMine(std::unordered_set<Unit> wk, std::unordered_set<Unit> mn) {
        for (Unit unit : wk) {
            if (distribution.find(unit) != distribution.end()) continue;
            int min_worker = -1;
            for (Unit mine : mn) {
                int count = worker_count[mine];
                if (min_worker == -1 || count < min_worker) min_worker = count;
            }
            Unit choose = unit;
            int min_dist = -1;
            for (Unit mine : mn) {
                if (worker_count[mine] != min_worker) continue;

                int dist = unit->getDistance(mine);
                if (min_dist == -1 || dist < min_dist) {
                    min_dist = dist;
                    choose = mine;
                }
            }
            if (choose == unit) continue;
            worker_count[choose]++;
            distribution[unit] = choose;
        }
	}
    void Gather(Unit unit) {
        unit->gather(distribution[unit]);
    }
};