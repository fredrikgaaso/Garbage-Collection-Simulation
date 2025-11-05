#include <omnetpp.h>
#include "message_m.h"

using namespace omnetpp;

class Can : public cSimpleModule
{
  private:
    bool hasGarbage = false;
    int lostCount = 0;
    std::string mode;
    int canId = 1;
    double interactionRadius = 450.0;
    double posX = 0.0, posY = 0.0;

    cOvalFigure *radiusFigure = nullptr;
  protected:
    virtual void initialize() override;
    virtual void handleMessage(cMessage *msg) override;
    void createOrUpdateRadiusFigure() {
                if (!getParentModule() || !getParentModule()->getCanvas())
                    return;

                auto *canvas = getParentModule()->getCanvas();

                if (!radiusFigure) {
                    radiusFigure = new cOvalFigure(("commRadius-" + std::string(getName())).c_str());
                    radiusFigure->setLineColor(cFigure::BLUE);
                    radiusFigure->setLineWidth(1);
                    radiusFigure->setFillColor(cFigure::BLUE);
                    radiusFigure->setFillOpacity(0.0);   // faint filled circle
                    canvas->addFigure(radiusFigure);
                }

                // Center the oval at (posX, posY) with radius interactionRadius
                const double r = interactionRadius;
                cFigure::Rectangle bounds(posX - r, posY - r, 2*r, 2*r);
                radiusFigure->setBounds(bounds);
            }
};

Define_Module(Can);

void Can::initialize()
{
    hasGarbage = par("hasGarbage").boolValue();
    mode = par("mode").stdstringValue();
    canId = par("canId").intValue();

    EV << "Can initialized. hasGarbage=" << hasGarbage << " mode=" << mode << "\n";

    // read position from NED parameters
    posX = par("x").doubleValue();
    posY = par("y").doubleValue();

    // optionally allow radius to be set from NED as a parameter too
    if (hasPar("range"))
        interactionRadius = par("range").doubleValue();

    EV << "Can init: id=" << canId
       << " pos=(" << posX << "," << posY << ")"
       << " radius=" << interactionRadius
       << " mode=" << mode << "\n";

    createOrUpdateRadiusFigure();




}

void Can::handleMessage(cMessage *msg)
{
    GarbageMessage *gm = check_and_cast<GarbageMessage *>(msg);
    EV << "Can received: " << gm->getName() << "\n";

    if ((gm->getId() == 1 || gm->getId() == 4) && lostCount < 3) {
        lostCount++;
        EV << "Simulated loss (" << lostCount << ")\n";
        bubble("message lost");
        delete gm;
        return;
    }
    bool smartphoneQuery = (gm->getId() == 1 || gm->getId() == 4);

    if (smartphoneQuery) {
        bool isFirstCan = (canId == 1);
        const char *yesName = isFirstCan ? "3-YES" : "6-YES";
                const char *noName = isFirstCan ? "2-NO" : "5-NO";
                const char *collectName = isFirstCan ? "7-Collect garbage" : "9-Collect garbage";
                int yesId = isFirstCan ? 3 : 6;
                int noId = isFirstCan ? 2 : 5;
                int collectId = isFirstCan ? 7 : 9;

            if (hasGarbage) {
                auto *reply = new GarbageMessage(yesName);
                reply->setId(yesId);
                reply->setText("YES");
                reply->setIsAck(false);
                reply->setSentTime(simTime().dbl());
                send(reply, "out");

                if (mode == "fast") {
                    auto *collect = new GarbageMessage(collectName);
                    collect->setId(collectId);
                    collect->setText("Collect garbage (fog)");
                    collect->setIsAck(false);
                    collect->setSentTime(simTime().dbl());

                    send(collect, "outCloud");
                }

            } else {
                auto *reply = new GarbageMessage(noName);
                reply->setId(noId);
                reply->setText("NO");
                reply->setIsAck(false);
                reply->setSentTime(simTime().dbl());
                send(reply, "out");
            }
        }

    delete gm;
}
