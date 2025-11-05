#include <omnetpp.h>
#include <string>
#include <vector>
#include <cmath>
#include <omnetpp/sim_std_m.h>
#include <cstdlib>
#include "message_m.h"

using namespace omnetpp;

namespace {
constexpr double PI = 3.14159265358979323846;
}

enum Peer { CAN1 = 0, CAN2 = 1, CLOUD = 2 };

class Smartphone : public cSimpleModule
{
  private:
    std::string mode;
    simtime_t retryInterval = 0.5;
    static const int MAX_TRIES = 4;

    double travelTime;

    int tries[2] = {0,0};
    bool waiting[2] = {true,true};
    bool yes[2] = {false,false};


    cMessage *tCan1 = nullptr;
    cMessage *tCan2 = nullptr;
    cMessage *mobilityTimer = nullptr;

    double numberOfMessagesSent;

    struct Coord {
          double x = 0;
          double y = 0;
      };

      struct Segment {
          Coord start;
          Coord end;
          simtime_t duration;
      };
    simtime_t mobilityUpdateInterval = 0.1;
    std::vector<Segment> mobilitySegments;
    size_t currentSegmentIndex = 0;
    simtime_t segmentStartTime = SIMTIME_ZERO;
    bool mobilityActive = false;
    Coord startPosition;
    bool hasStartPosition = false;
    Coord currentPosition;
    cOvalFigure *radiusFigure = nullptr;
    Coord canPositions[2];
    bool canActivationScheduled[2] = {false, false};
    bool interactionDone[2] = {false, false};
    bool collectSent[2] = {false, false};
    bool canPositionValid[2] = {false, false};
    double interactionRadius = 400.0;
    bool mobilityPaused = false;
    int pauseForCan = -1;
    simtime_t pauseStartTime = SIMTIME_ZERO;

    cRectangleFigure *box = nullptr;
    cTextFigure *title = nullptr;
    cTextFigure *body = nullptr;
    cTextFigure *statusText = nullptr;

    double statusOffsetY = -200.0;
    double statusOffsetX = -400;

    const double X = 2600, Y = 100;
    const double W = 780,  H = 1350;

    int sentTxt = 0, rcvdTxt = 0;

    int sentFastTxt = 0, sentSlowTxt = 0, rcvdFastTxt = 0, rcvdSlowTxt = 0;



    double msPhoneToCan1 = -1, msCan1ToPhone = -1;
    double msPhoneToCan2 = -1, msCan2ToPhone = -1;
    double msPhoneToCloud = -1, msCloudToPhone = -1;
    double msCan1ToCloud = -1, msCloudToCan1 = -1;
    double msCan2ToCloud = -1, msCloudToCan2 = -1;



    std::string pickBody(const std::string& mode) {
        if (mode == "fast") {
            return
                    "Slow connection from the smartphone to others (time it takes) = 0 \n"
                    "Slow connection from others to the smartphone (time it takes) = 0\n"
                    "Fast connection from the smartphone to others (time it takes) = \n"
                    "Fast connection from others to the smartphone (time it takes) = \n\n"
                    "Connection from the can to others (time it takes) =\n"
                                        "Connection from others to the can (time it takes) =\n\n"
                                        "Connection from the anotherCan to others (time it takes) =\n"
                                        "Connection from others to the anotherCan (time it takes) =\n\n"
                                        "Slow connection from the Cloud to others (time it takes) =\n"
                                        "Slow connection from others to the Cloud (time it takes) =\n"
                                        "Fast connection from the Cloud to others (time it takes) =0\n"
                                        "Fast connection from others to the Cloud (time it takes) =0";
            ;

        } else if (mode == "slow") {
            return
                    "Slow connection from the smartphone to others (time it takes) =\n"
                    "Slow connection from others to the smartphone (time it takes) =\n"
                    "Fast connection from the smartphone to others (time it takes) = 0\n"
                    "Fast connection from others to the smartphone (time it takes) = 0\n"
                    "Connection from the can to others (time it takes) =\n"
                    "Connection from others to the can (time it takes) =\n\n"
                    "Connection from the anotherCan to others (time it takes) =\n"
                    "Connection from others to the anotherCan (time it takes) =\n\n"
                    "Slow connection from the Cloud to others (time it takes) =\n"
                    "Slow connection from others to the Cloud (time it takes) =\n"
                    "Fast connection from the Cloud to others (time it takes) =0\n"
                    "Fast connection from others to the Cloud (time it takes) =0";
        } else { // "none"
            return
                    "Slow connection from the smartphone to others (time it takes) =\n"
                    "Slow connection from others to the smartphone (time it takes) =\n"
                    "Fast connection from the smartphone to others (time it takes) =\n"
                    "Fast connection from others to the smartphone (time it takes) =\n\n"
                    "Connection from the can to others (time it takes) =\n"
                    "Connection from others to the can (time it takes) =\n\n"
                    "Connection from the anotherCan to others (time it takes) =\n"
                    "Connection from others to the anotherCan (time it takes) =\n\n"
                    "Slow connection from the Cloud to others (time it takes) = 0\n"
                    "Slow connection from others to the Cloud (time it takes) = 0\n"
                    "Fast connection from the Cloud to others (time it takes) = 0\n"
                    "Fast connection from others to the Cloud (time it takes) = 0";
        }
      }


  protected:
    virtual ~Smartphone() override;
    virtual void initialize() override;
    virtual void handleMessage(cMessage *msg) override;
    void updateLegend();
    void sendQuery(int whichCan);
    void setupMobility();
    void parseMobilityDefinition(cXMLElement *root);
    void buildDefaultRoute();
    void advanceMobility();
    void setDisplayPosition(const Coord& coord);
    void activateCan(int whichCan);
    void maybeTriggerCanActivation();
    double distanceTo(const Coord& a, const Coord& b) const;
    void resumeMobilityIfPaused(int whichCan);
    void updateRadiusFigure(const Coord& coord);
    bool loadCanPosition(const char *moduleName, int whichCan);
    void finish();
    void updateStatusText();
};

Define_Module(Smartphone);

void Smartphone::initialize()
{
    mode = par("mode").stdstringValue();

    tCan1 = new cMessage("tCan1");
    tCan2 = new cMessage("tCan2");

    updateLegend();


    cModule *can1 = getParentModule()->getSubmodule("can1");
    double x1 = can1->par("x").doubleValue();
    double y1 = can1->par("y").doubleValue();

    cModule *can2 = getParentModule()->getSubmodule("can2");
    double x2 = can2->par("x").doubleValue();
    double y2 = can2->par("y").doubleValue();

    tries[CAN1] = tries[CAN2] = 0;
    waiting[CAN1] = waiting[CAN2] = false;
    yes[CAN1] = yes[CAN2] = false;
    interactionDone[CAN1] = interactionDone[CAN2] = false;
    canActivationScheduled[CAN1] = canActivationScheduled[CAN2] = false;
    collectSent[CAN1] = collectSent[CAN2] = false;

    bool fallbackActivation[2] = {false, false};
    // fetch can display positions to align interaction checkpoints
    if (!loadCanPosition("can1", CAN1)) {
        fallbackActivation[CAN1] = true;
    }
    if (!loadCanPosition("can2", CAN2)) {
        fallbackActivation[CAN2] = true;
    }

    mobilityUpdateInterval = par("mobilityUpdateInterval").doubleValue();

    if (auto *parent = getParentModule()) {
        if (auto *canvas = parent->getCanvas()) {
            radiusFigure = new cOvalFigure("communicationRadius");
            radiusFigure->setLineColor(cFigure::BLUE);
            radiusFigure->setLineWidth(1);
            radiusFigure->setFillColor(cFigure::BLUE);
            radiusFigure->setFillOpacity(0.0);
            canvas->addFigure(radiusFigure);



                   // title
                   title = new cTextFigure("legendTitle");
                   title->setText(mode == "fast" ? "Fog-based (fast)"
                                   : mode == "slow" ? "Cloud-based (slow)"
                                   : "No-garbage");
                   title->setFont(cFigure::Font("Arial", 50));
                   title->setColor(cFigure::BLACK);
                   title->setPosition(cFigure::Point(X + 20, Y + 20));
                   canvas->addFigure(title);

                   // body text (multi-line)
                   body = new cTextFigure("legendBody");
                   body->setText(pickBody(mode).c_str());
                   body->setFont(cFigure::Font("Arial", 50));
                   body->setColor(cFigure::BLACK);
                   body->setPosition(cFigure::Point(X + 20, Y + 70));
                   canvas->addFigure(body);

                   statusText = new cTextFigure(("status-" + std::string(getName())).c_str());
                        statusText->setFont(cFigure::Font("Arial", 50));
                        statusText->setColor(cFigure::BLUE);
                        statusText->setPosition(cFigure::Point(0, 0 ));
                        canvas->addFigure(statusText);
                        updateStatusText();
        }
    }

    setupMobility();

    // ensure can positions are known and valid (matching Network.ned layout)
    canPositions[CAN1].x = x1;
    canPositions[CAN1].y = y1;
    canPositionValid[CAN1] = true;
    canPositions[CAN2].x = x2;
    canPositions[CAN2].y = y2;
    canPositionValid[CAN2] = true;

        maybeTriggerCanActivation();

}

bool Smartphone::loadCanPosition(const char *moduleName, int whichCan)
{
    if (!moduleName || whichCan < CAN1 || whichCan > CAN2)
        return false;

    cModule *parent = getParentModule();
    if (!parent)
        return false;

    cModule *target = parent->getSubmodule(moduleName);
    if (!target)
        return false;

    const char *x = target->getDisplayString().getTagArg("p", 0);
    const char *y = target->getDisplayString().getTagArg("p", 1);
    if (!(x && *x) || !(y && *y))
        return false;

    canPositions[whichCan].x = atof(x);
    canPositions[whichCan].y = atof(y);
    canPositionValid[whichCan] = true;
    return true;
}

void Smartphone::sendQuery(int whichCan)
{


    if (!waiting[whichCan]) {
        EV << "[DEBUG] sendQuery REJECTED: not waiting\n";
        return;
    }
    if (tries[whichCan] >= MAX_TRIES) {
        EV << "[DEBUG] sendQuery REJECTED: max tries reached\n";
        return;
    }

    auto *msg = new GarbageMessage(whichCan == CAN1 ? "1-Is the can full?" : "4-Is the can full?");
    msg->setId(whichCan == CAN1 ? 1 : 4);
    msg->setText("Is the can full?");
    msg->setIsAck(false);
    msg->setSentTime(simTime().dbl());
    send(msg, "out", whichCan);
    numberOfMessagesSent++;
    sentSlowTxt++;
    updateStatusText();


    tries[whichCan]++;
}

void Smartphone::updateLegend() {
    if (!body) return;

    auto fmt = [](double vms) {
        if (vms < 0) return std::string("0");        // not yet measured -> show 0 (like the videos)
        char b[32]; std::snprintf(b, sizeof(b), "%.2f ms", vms);
        return std::string(b);
    };

    // Map the professor's generic phrasing to your links:
    // "Slow connection from the smartphone to others"  -> BLE Smartphone -> Can
    // "Slow connection from others to the smartphone"  -> BLE Can -> Smartphone
    // "Fast connection from the smartphone to others"  -> Smartphone -> Cloud (slow mode) OR 0 in fast
    // "Fast connection from others to the smartphone"  -> Cloud -> Smartphone

    std::ostringstream ss;
    ss.setf(std::ios::fixed); ss.precision(2);

    // Slow side (BLE)
    ss << "Slow connection from the smartphone to others (time it takes) = " << fmt(msPhoneToCan1 + msPhoneToCan2 * numberOfMessagesSent) << "\n"
       << "Slow connection from others to the smartphone (time it takes) = " << fmt(msCan1ToPhone + msCan2ToPhone) << "\n";

    // Fast side (WAN). In slow mode: phone<->cloud; in fast mode: cloud->phone only
    if (mode == "slow") {
        ss << "Fast connection from the smartphone to others (time it takes) = " << fmt(msPhoneToCloud) << "\n";
    } else {
        ss << "Fast connection from the smartphone to others (time it takes) = 0\n";
    }
    ss << "Fast connection from others to the smartphone (time it takes) = " << fmt(msCloudToPhone) << "\n\n";

    // Extra lines for cans/cloud as in the professor’s layout
    // Use the same measured numbers for both cans, or keep separate if you track per-can.
    ss << "Connection from the can to others (time it takes) = " << fmt(msCan1ToCloud + msCan1ToPhone) << "\n"
       << "Connection from others to the can (time it takes) = " << fmt(msCloudToCan1 + msPhoneToCan1) << "\n\n"
       << "Connection from the anotherCan to others (time it takes) = " << fmt(msCan2ToCloud + msCan2ToPhone) << "\n"
       << "Connection from others to the anotherCan (time it takes) = " << fmt(msCloudToCan2 + msPhoneToCan2) <<"\n\n"


       << "Slow connection from the Cloud to others (time it takes) = "<< fmt(msCloudToCan1 + msCloudToCan2) << "\n"
       << "Slow connection from others to the Cloud (time it takes) = " << fmt(msCan1ToCloud + msCan2ToCloud) << "\n"
       << "Fast connection from the Cloud to others (time it takes) = "<< fmt(msCloudToPhone) <<"\n"
       << "Fast connection from others to the Cloud (time it takes) = " << fmt(msPhoneToCloud);

    body->setText(ss.str().c_str());
}

void Smartphone::handleMessage(cMessage *msg)
{

    if (msg == mobilityTimer) {
        advanceMobility();
        return;
    }

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

    const cGate *arrivalGate = gm->getArrivalGate();
    int arrivalGateIndex = arrivalGate ? arrivalGate->getIndex() : -1;
    const char *arrivalGateName = arrivalGate ? arrivalGate->getFullName() : "<none>";

    EV << "[SMARTPHONE MSG] " << gm->getName()
       << " id=" << gm->getId()
       << " isAck=" << (gm->isAck() ? "true" : "false")
       << " gate=" << arrivalGateName << "(" << arrivalGateIndex << ")\n";

    travelTime = simTime().dbl() - gm->getSentTime();
    rcvdTxt++;
    updateStatusText();

    EV << "[DELAY] " << gm->getName() << " took "
       << (travelTime * 1000) << " ms to arrive.\n";
    if (arrivalGateIndex == CAN1) {
          msPhoneToCan1 += gm->getPrevHopDelay() * 1000;
          msCan1ToPhone += travelTime * 1000;
          EV << "[DELAY] Can->Phone = " << msCan1ToPhone
             << " ms | Phone->Can = " << msPhoneToCan1 << " ms\n";
      } else if ( arrivalGateIndex == CAN2){
          msPhoneToCan2 += gm->getPrevHopDelay() * 1000;
          msCan2ToPhone += travelTime * 1000;
          EV << "[DELAY] Can->Phone = " << msCan2ToPhone
                    << " ms | Phone->Can = " << msPhoneToCan2 << " ms\n";
      }


    EV << "previous hop delay: " << msPhoneToCan1;
    switch (gm->getId()) {
        case 2:
        case 3:
        case 5:
        case 6:
            if (gm->getFromCloud()) {
                if (gm->getId() == 3) {
                    msCan1ToCloud += gm->getPrevHopDelay() * 1000;
                    msCloudToCan1 += travelTime * 1000;
                    EV << "Triggered ID 3 (Can1 <-> Cloud)"
                       << " | Can→Cloud: " << msCan1ToCloud
                       << " | Cloud→Can: " << msCloudToCan1 << "\n";
                } else { // id == 6
                    msCan2ToCloud += gm->getPrevHopDelay() * 1000;
                    msCloudToCan2 += travelTime * 1000;
                    EV << "Triggered ID 6 (Can2 <-> Cloud)"
                       << " | Can→Cloud: " << msCan2ToCloud
                       << " | Cloud→Can: " << msCloudToCan2 << "\n";
                }
                rcvdSlowTxt++;
                updateStatusText();
            } else {
                rcvdSlowTxt++;
                updateStatusText();
            }
            break;

        case 8:
        case 10:
            msCloudToPhone += travelTime * 1000;
            msPhoneToCloud += gm->getPrevHopDelay() * 1000;
            rcvdFastTxt++;
            updateStatusText();
            EV << "Triggered ID " << gm->getId()
               << " (Cloud <-> Phone)"
               << " | Cloud→Phone: " << msCloudToPhone
               << " | Phone→Cloud: " << msPhoneToCloud << "\n";
            break;


        default:
            break;
    }



    if (arrivalGateIndex == CAN1) {
        waiting[CAN1] = false;
        yes[CAN1] = (gm->getId() == 3);
        interactionDone[CAN1] = true;
        if (tCan1->isScheduled())
            cancelEvent(tCan1);
        resumeMobilityIfPaused(CAN1);
    } else if (arrivalGateIndex == CAN2) {
        waiting[CAN2] = false;
        yes[CAN2] = (gm->getId() == 6);
        interactionDone[CAN2] = true;
        if (tCan2->isScheduled())
            cancelEvent(tCan2);
        resumeMobilityIfPaused(CAN2);
    }

    if (mode == "slow") {
        EV << "[SMARTPHONE COLLECT] flags can1=" << collectSent[CAN1] << " can2=" << collectSent[CAN2] << "\n";
        if (arrivalGateIndex == CAN1 && gm->getId() == 3 && !collectSent[CAN1]) {
            auto *collect = new GarbageMessage("7-Collect garbage");
            collect->setId(7);
            collect->setText("Collect garbage");
            collect->setIsAck(false);
            collectSent[CAN1] = true;
            collect->setSentTime(simTime().dbl());
            collect->setFromCloud(true);
            EV << "Smartphone sending 7-Collect garbage\n";
            send(collect, "out", CLOUD);
            sentFastTxt++;
            updateStatusText();



        } else if (arrivalGateIndex == CAN2 && gm->getId() == 6 && !collectSent[CAN2]) {
            auto *collect = new GarbageMessage("9-Collect garbage");
            collect->setId(9);
            collect->setText("Collect garbage");
            collect->setIsAck(false);
            collectSent[CAN2] = true;
            EV << "Smartphone sending 9-Collect garbage\n";
            collect->setFromCloud(true);
            collect->setSentTime(simTime().dbl());

            send(collect, "out", CLOUD);
            sentFastTxt++;
            updateStatusText();


        }
    }
    delete gm;
}

Smartphone::~Smartphone()
{
    if (tCan1)
        cancelAndDelete(tCan1);
    if (tCan2)
        cancelAndDelete(tCan2);
    if (mobilityTimer)
        cancelAndDelete(mobilityTimer);
    if (radiusFigure) {
        if (auto *parentFigure = radiusFigure->getParentFigure())
            parentFigure->removeFigure(radiusFigure);
        delete radiusFigure;
        radiusFigure = nullptr;
    }
}

void Smartphone::setupMobility()
{
    cXMLElement *config = par("mobilityConfig").xmlValue();
    mobilitySegments.clear();
    hasStartPosition = false;

    if (config) {
        parseMobilityDefinition(config);
    }

    if (!hasStartPosition || mobilitySegments.empty()) {
        EV_WARN << "Mobility script missing or empty, falling back to built-in route\n";
        buildDefaultRoute();
    }

    if (hasStartPosition)
        setDisplayPosition(startPosition);

    if (mobilitySegments.empty()) {
        EV_WARN << "No mobility segments defined; smartphone will remain stationary.\n";
        return;
    }

    if (!mobilityTimer)
        mobilityTimer = new cMessage("mobilityTimer");
    else
        cancelEvent(mobilityTimer);

    mobilityActive = true;
    currentSegmentIndex = 0;
    segmentStartTime = simTime();

    if (mobilityActive)
        scheduleAt(simTime(), mobilityTimer);
}

void Smartphone::parseMobilityDefinition(cXMLElement *root)
{
    Coord currentPosition;
    double currentAngleDeg = 0;
    double currentSpeed = 0;
    bool positionInitialized = false;

    for (cXMLElement *movement = root->getFirstChild(); movement; movement = movement->getNextSibling()) {
        if (strcmp(movement->getTagName(), "movement") != 0)
            continue;

        for (cXMLElement *cmd = movement->getFirstChild(); cmd; cmd = cmd->getNextSibling()) {
            const char *tag = cmd->getTagName();
            if (strcmp(tag, "set") == 0) {
                if (const char *xAttr = cmd->getAttribute("x"))
                    currentPosition.x = atof(xAttr);
                if (const char *yAttr = cmd->getAttribute("y"))
                    currentPosition.y = atof(yAttr);
                if (const char *angleAttr = cmd->getAttribute("angle"))
                    currentAngleDeg = atof(angleAttr);
                if (const char *speedAttr = cmd->getAttribute("speed"))
                    currentSpeed = atof(speedAttr);

                if (!positionInitialized) {
                    startPosition = currentPosition;
                    hasStartPosition = true;
                    positionInitialized = true;
                }
            } else if (strcmp(tag, "turn") == 0) {
                if (const char *angleAttr = cmd->getAttribute("angle")) {
                    currentAngleDeg = std::fmod(currentAngleDeg + atof(angleAttr), 360.0);
                    if (currentAngleDeg < 0)
                        currentAngleDeg += 360.0;
                }
            } else if (strcmp(tag, "forward") == 0) {
                if (!positionInitialized || currentSpeed <= 0)
                    continue;

                const char *distAttr = cmd->getAttribute("d");
                if (!distAttr)
                    continue;

                double distance = atof(distAttr);
                Coord start = currentPosition;
                double radians = currentAngleDeg * PI / 180.0;
                currentPosition.x += std::cos(radians) * distance;
                currentPosition.y += std::sin(radians) * distance;
                simtime_t duration = currentSpeed > 0 ? SimTime(distance / currentSpeed) : SimTime();

                Segment seg;
                seg.start = start;
                seg.end = currentPosition;
                seg.duration = duration;
                mobilitySegments.push_back(seg);
            }
        }
    }

    if (!hasStartPosition && positionInitialized) {
        startPosition = currentPosition;
        hasStartPosition = true;
    }
}

void Smartphone::buildDefaultRoute()
{
    mobilitySegments.clear();

    startPosition = {2190.0, 420.0};
    hasStartPosition = true;

    double speed = 340.0;

    auto addSegment = [&](const Coord& from, const Coord& to) {
        Segment seg;
        seg.start = from;
        seg.end = to;
        double dist = distanceTo(from, to);
        seg.duration = speed > 0 ? SimTime(dist / speed) : SimTime();
        mobilitySegments.push_back(seg);
    };

    Coord p1 = startPosition;
    Coord p2 = {540.0, 420.0};
    Coord p3 = {540.0, 1280.0};
    Coord p4 = {2190.0, 1280.0};

    addSegment(p1, p2);
    addSegment(p2, p3);
    addSegment(p3, p4);
}

void Smartphone::advanceMobility()
{
    if (!mobilityActive) {
        return;
    }

    if (mobilityPaused) {
        return;
    }

    if (currentSegmentIndex >= mobilitySegments.size()) {
        mobilityActive = false;
        return;
    }

    Segment &segment = mobilitySegments[currentSegmentIndex];
    simtime_t elapsed = simTime() - segmentStartTime;

    if (segment.duration <= SIMTIME_ZERO || elapsed >= segment.duration) {
        setDisplayPosition(segment.end);
        EV << "Movement reached segment end (" << segment.end.x << "," << segment.end.y << ")\n";
        currentSegmentIndex++;
        segmentStartTime = simTime();

        if (currentSegmentIndex >= mobilitySegments.size()) {
            mobilityActive = false;
        }
    } else {
        double fraction = segment.duration.dbl() == 0 ? 1.0 : elapsed.dbl() / segment.duration.dbl();
        Coord interpolated;
        interpolated.x = segment.start.x + (segment.end.x - segment.start.x) * fraction;
        interpolated.y = segment.start.y + (segment.end.y - segment.start.y) * fraction;
        setDisplayPosition(interpolated);
    }

    maybeTriggerCanActivation();

    if (mobilityActive && !mobilityPaused) {
        scheduleAt(simTime() + mobilityUpdateInterval, mobilityTimer);
    }
}

void Smartphone::activateCan(int whichCan)
{
    EV << "[DEBUG] activateCan CALLED: whichCan=" << whichCan
       << " interactionDone=" << interactionDone[whichCan]
       << " waiting=" << waiting[whichCan] << " at t=" << simTime() << "\n";

    if (whichCan < 0 || whichCan > 1) {
        EV << "[DEBUG] activateCan REJECTED: invalid whichCan\n";
        return;
    }
    if (interactionDone[whichCan] || waiting[whichCan]) {
        EV << "[DEBUG] activateCan REJECTED: already done or waiting\n";
        return;
    }

    EV << "[DEBUG] activateCan PROCEEDING: whichCan=" << whichCan << " at t=" << simTime() << "\n";

    cMessage *timer = whichCan == CAN1 ? tCan1 : tCan2;
    waiting[whichCan] = true;
    tries[whichCan] = 0;
    yes[whichCan] = false;
    canActivationScheduled[whichCan] = true;

    if (!timer->isScheduled()) {
        scheduleAt(simTime() + 0.1, timer);
    }

    if (mobilityActive && (!mobilityPaused || pauseForCan == -1)) {
        mobilityPaused = true;
        pauseForCan = whichCan;
        pauseStartTime = simTime();
    }
}

void Smartphone::maybeTriggerCanActivation()
{
    for (int i = 0; i < 2; ++i) {
        if (!canPositionValid[i])
            continue;
        double d = distanceTo(currentPosition, canPositions[i]);
        if (interactionDone[i] || canActivationScheduled[i] || !canPositionValid[i])
            continue;

        if (i == CAN2 && !interactionDone[CAN1])
            continue;

        if (d <= interactionRadius) {
            canActivationScheduled[i] = true;
            EV << "[DEBUG] within radius: activating can" << (i+1) << " distance=" << d << "\n";
            activateCan(i);
        }
    }
}

double Smartphone::distanceTo(const Coord& a, const Coord& b) const
{
    double dx = a.x - b.x;
    double dy = a.y - b.y;
    return std::sqrt(dx * dx + dy * dy);
}

void Smartphone::resumeMobilityIfPaused(int whichCan)
{
    if (!mobilityPaused || pauseForCan != whichCan)
        return;

    simtime_t pauseDuration = simTime() - pauseStartTime;
    pauseStartTime = SIMTIME_ZERO;
    mobilityPaused = false;
    pauseForCan = -1;

    segmentStartTime += pauseDuration;

    if (mobilityActive && mobilityTimer && !mobilityTimer->isScheduled()) {
        scheduleAt(simTime() + mobilityUpdateInterval, mobilityTimer);
    }
}

void Smartphone::setDisplayPosition(const Coord& coord)
{
    currentPosition = coord;
    getDisplayString().setTagArg("p", 0, coord.x);
    getDisplayString().setTagArg("p", 1, coord.y);
    updateRadiusFigure(coord);
    if (statusText) {
           statusText->setPosition(cFigure::Point(coord.x + statusOffsetX, coord.y + statusOffsetY));
       }
}

void Smartphone::updateRadiusFigure(const Coord& coord)
{
    if (!radiusFigure)
        return;

    double diameter = interactionRadius * 2.0;
    cFigure::Rectangle bounds(coord.x - interactionRadius,
                              coord.y - interactionRadius,
                              diameter,
                              diameter);
    radiusFigure->setBounds(bounds);
}

void Smartphone::updateStatusText() {
    if (!statusText) return;
       char buf[64];
       std::snprintf(buf, sizeof(buf), "sentFast:%d  rcvdFast:%d sentSlow:%d rcvdSlow:%d", sentFastTxt, rcvdFastTxt, sentSlowTxt, rcvdSlowTxt);
       statusText->setText(buf);
}
void Smartphone::finish(){
    EV << numberOfMessagesSent;
    updateLegend();


}
