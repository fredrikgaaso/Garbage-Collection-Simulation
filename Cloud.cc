#include <omnetpp.h>
#include "message_m.h"
using namespace omnetpp;

class Cloud : public cSimpleModule {
  protected:
    virtual void handleMessage(cMessage *msg) override {
        auto *gm = check_and_cast<GarbageMessage *>(msg);
        EV << "Cloud received: " << gm->getName() << "\n";

        int idx = gm->getArrivalGate()->getIndex();
        auto *ok = new GarbageMessage(idx == 0 ? "8-OK" : "10-OK");
        ok->setId(idx == 0 ? 8 : 10);
        ok->setText("OK");
        ok->setIsAck(true);
        send(ok, "out", idx);

        delete gm;
    }
};
Define_Module(Cloud);
