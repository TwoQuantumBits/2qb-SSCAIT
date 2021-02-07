import bwapi.*;

import java.lang.Integer;

public class Bot extends DefaultBWListener {
    BWClient bwClient;
    Game game;

    public void onUnitComplete(Unit unit)
    {
        if (unit.getType().isWorker())
        {
            Unit closestMineral = null;
            int closestDistance = Integer.MAX_VALUE;
            for(Unit mineral : game.getMinerals())
            {
                int distance = unit.getDistance(mineral);
                if (distance < closestDistance)
                {
                    closestMineral = mineral;
                    closestDistance = distance;
                }
            }
            unit.gather(closestMineral);
        }
    }

    public void train(Player player)
    {
        for (Unit trainer : player.getUnits()) {
            UnitType unitType = trainer.getType();
            if (unitType.isBuilding() && !unitType.buildsWhat().isEmpty()) {
                UnitType toTrain = unitType.buildsWhat().get(0);
                if (game.canMake(toTrain, trainer)) {
                    trainer.train(toTrain);
                }
            }
        }
    }

    public void printUnits(Player player)
    {
        int lineIndex = 40;
        for (Unit unit : player.getUnits())
        {
            game.drawTextScreen(10, lineIndex, (unit.getType().name() + " (" + unit.getLeft() + ", " + unit.getBottom() + ")").replaceAll("_", " "));
            lineIndex += 10;
        }
    }

    @Override
    public void onStart() {
        game = bwClient.getGame();
    }

    @Override
    public void onFrame() {
        Player self = game.self();
        game.drawTextScreen(10, 10, "Playing as " + self.getName() + " - " + self.getRace());
        game.drawTextScreen(10, 20, "Resources: " + self.minerals() + " minerals,  " + self.gas() + " gas");
        game.drawTextScreen(10, 30, "Units:");
        printUnits(self);
        train(self);
    }

    void run() {
        bwClient = new BWClient(this);
        bwClient.startGame();
    }

    public static void main(String[] args) {
        new Bot().run();
    }
}