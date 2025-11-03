#include <omnetpp.h>
#include "message_m.h"

using namespace omnetpp;

class Can : public cSimpleModule
{
  private:
    bool hasGarbage = false;
    int lostCount = 0;
    std::string mode;
  protected:
    virtual void initialize() override;
    virtual void handleMessage(cMessage *msg) override;
};

Define_Module(Can);

void Can::initialize()
{
    hasGarbage = par("hasGarbage").boolValue();
    mode = par("mode").stdstringValue();
    EV << "Can initialized. hasGarbage=" << hasGarbage << " mode=" << mode << "\n";
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

    if (gm->getId() == 1 || gm->getId() == 4) {
            GarbageMessage *reply = nullptr;
            if (hasGarbage) {
                reply = new GarbageMessage("3-YES");
                reply->setId(3);
                reply->setText("YES");
                reply->setIsAck(false);
                send(reply, "out");

                if (mode == "fast") {
                    auto *collect = new GarbageMessage("7-Collect garbage");
                    collect->setId(7);
                    collect->setText("Collect garbage (fog)");
                    collect->setIsAck(false);
                    send(collect, "outCloud");
                }

            } else if (!hasGarbage) {
                reply = new GarbageMessage("2-NO");
                reply->setId(2);
                reply->setText("NO");
                reply->setIsAck(false);
                send(reply, "out");
            }
        }

    delete gm;
}
