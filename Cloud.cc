#include <omnetpp.h>
#include "message_m.h"
using namespace omnetpp;

class Cloud : public cSimpleModule {
private:
    int sentFastTxt = 0, rcvdFastTxt = 0, sentSlowTxt = 0, rcvdSlowTxt = 0;
    cTextFigure *statusText = nullptr;

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
                          radiusFigure->setFillOpacity(0.0);
                          canvas->addFigure(radiusFigure);
                      }

                      const double r = interactionRadius;
                      cFigure::Rectangle bounds(posX - r, posY - r, 2*r, 2*r);
                      radiusFigure->setBounds(bounds);
                  }

    virtual void handleMessage(cMessage *msg) override {

        auto *gm = check_and_cast<GarbageMessage *>(msg);
        EV << "Cloud received: " << gm->getName() << "\n";
        double inbound = simTime().dbl() - gm->getSentTime();

        if(gm->getFromCloud() == true) {
            rcvdFastTxt++;
            updateStatusText();

        }else {
            rcvdSlowTxt++;
            updateStatusText();

        }


        int idx = gm->getArrivalGate()->getIndex();
        int msgId = gm->getId();
        const char *ackName = msgId == 7 ? "8-OK" : "10-OK";
           int ackId = msgId == 7 ? 8 : 10;

           auto *ok = new GarbageMessage(ackName);
                   ok->setId(ackId);
                   ok->setText("OK");
                   ok->setIsAck(true);
                   ok->setFromCloud(true);
                   ok->setPrevHopDelay(inbound);
                   ok->setSentTime(simTime().dbl());
                   if(gm->getFromCloud() == true) {
                       sentFastTxt++;
                       updateStatusText();

                   }else {
                       sentSlowTxt++;
                       updateStatusText();

                   }
                   send(ok, "out", idx);


        delete gm;
    }
    void updateStatusText() {
        if (!statusText) return;
        char buf[64];
        std::snprintf(buf, sizeof(buf), "sentFast:%d  rcvdFast:%d sentSlow:%d rcvdSlow:%d",  sentSlowTxt, rcvdSlowTxt, sentFastTxt, rcvdFastTxt);
        statusText->setText(buf);
    }
};
Define_Module(Cloud);

void Cloud::initialize()
{


    posX = par("x").doubleValue();
    posY = par("y").doubleValue();

    if (hasPar("range"))
        interactionRadius = par("range").doubleValue();

    createOrUpdateRadiusFigure();

    if (auto *canvas = getParentModule()->getCanvas()) {
        statusText = new cTextFigure(("status-" + std::string(getName())).c_str());
        statusText->setFont(cFigure::Font("Arial", 50));
        statusText->setColor(cFigure::BLUE);
        statusText->setPosition(cFigure::Point(posX - 400, posY - 150));
        canvas->addFigure(statusText);
        updateStatusText();
    }



}
