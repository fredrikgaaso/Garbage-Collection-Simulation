#include <omnetpp.h>
#include "message_m.h"
using namespace omnetpp;

class Cloud : public cSimpleModule {
private:
    double interactionRadius = 1500.0;
         double posX = 0.0, posY = 0.0;

         cOvalFigure *radiusFigure = nullptr;
  protected:
         virtual void initialize() override;
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

    virtual void handleMessage(cMessage *msg) override {
        auto *gm = check_and_cast<GarbageMessage *>(msg);
        EV << "Cloud received: " << gm->getName() << "\n";



        int idx = gm->getArrivalGate()->getIndex();
        int msgId = gm->getId();
        const char *ackName = msgId == 7 ? "8-OK" : "10-OK";
           int ackId = msgId == 7 ? 8 : 10;

           auto *ok = new GarbageMessage(ackName);
                   ok->setId(ackId);
                   ok->setText("OK");
                   ok->setIsAck(true);
                   ok->setSentTime(simTime().dbl());
                   send(ok, "out", idx);


        delete gm;
    }
};
Define_Module(Cloud);

void Cloud::initialize()
{


    // read position from NED parameters
    posX = par("x").doubleValue();
    posY = par("y").doubleValue();

    // optionally allow radius to be set from NED as a parameter too
    if (hasPar("range"))
        interactionRadius = par("range").doubleValue();

    createOrUpdateRadiusFigure();




}
