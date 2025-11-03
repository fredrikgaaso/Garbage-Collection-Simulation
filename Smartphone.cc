#include <omnetpp.h>
#include <string>
#include "message_m.h"

using namespace omnetpp;

enum Peer { CAN1 = 0, CAN2 = 1, CLOUD = 2 };

class Smartphone : public cSimpleModule
{
  private:
    std::string mode;
    simtime_t retryInterval = 0.5;
    static const int MAX_TRIES = 4;

    int tries[2] = {0,0};
    bool waiting[2] = {true,true};
    bool yes[2] = {false,false};

    cMessage *tCan1 = nullptr;
    cMessage *tCan2 = nullptr;

  protected:
    virtual void initialize() override;
    virtual void handleMessage(cMessage *msg) override;

    void startPolling();
    void sendQuery(int whichCan);
};

Define_Module(Smartphone);

void Smartphone::initialize()
{
    mode = par("mode").stdstringValue();
    EV << "Smartphone mode: " << mode << "\n";

    tCan1 = new cMessage("tCan1");
    tCan2 = new cMessage("tCan2");

    startPolling();
}

void Smartphone::startPolling()
{
    tries[CAN1] = tries[CAN2] = 0;
    waiting[CAN1] = waiting[CAN2];
    yes[CAN1] = yes[CAN2] = false;

    scheduleAt(simTime() + 0.1, tCan1);
    scheduleAt(simTime() + 0.1, tCan2);
}

void Smartphone::sendQuery(int whichCan)
{
    if (!waiting[whichCan]) return;
    if (tries[whichCan] >= MAX_TRIES) return;

    auto *msg = new GarbageMessage(whichCan == CAN1 ? "1-Is the can full?" : "4-Is the can full?");
    msg->setId(whichCan == CAN1 ? 1 : 4);
    msg->setText("Is the can full?");
    msg->setIsAck(false);
    send(msg, "out", whichCan);

    tries[whichCan]++;
}

void Smartphone::handleMessage(cMessage *msg)
{
    if (msg == tCan1) {
        sendQuery(CAN1);
        if (waiting[CAN1] && tries[CAN1] < MAX_TRIES)
            scheduleAt(simTime() + retryInterval, tCan1);
        return;
    }
    if (msg == tCan2) {
        sendQuery(CAN2);
        if (waiting[CAN2] && tries[CAN2] < MAX_TRIES)
            scheduleAt(simTime() + retryInterval, tCan2);
        return;
    }

    GarbageMessage *gm = check_and_cast<GarbageMessage *>(msg);
    EV << "Smartphone received: " << gm->getName()
       << " id=" << gm->getId()
       << " isAck=" << (gm->isAck() ? "true" : "false") << "\n";


    if (gm->getId() == 2 || gm->getId() == 3) {
        waiting[CAN1] = false;
        yes[CAN1] = (gm->getId() == 3);
    } else if (gm->getId() == 5 || gm->getId() == 6) {
        waiting[CAN2] = false;
        yes[CAN2] = (gm->getId() == 6);
    }

    if (mode == "slow" && (yes[CAN1] || yes[CAN2])) {
        auto *collect = new GarbageMessage("7-Collect garbage");
        collect->setId(7);
        collect->setText("Collect garbage");
        collect->setIsAck(false);
        send(collect, "out", CLOUD);
    }

    delete gm;
}
