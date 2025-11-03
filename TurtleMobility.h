
#include "inet/mobility/single/TurtleMobility.h"

namespace Extended{

class TurtleMobility : public inet::TurtleMobility
{

public:
    void setLeg(inet::cXMLElement * leg);
};


}
