channel SlowLink extends ned.DatarateChannel
{
    delay = 100ms;
    datarate = 1Mbps;
}
channel FastLink extends ned.DatarateChannel
{
    delay = 10ms;
    datarate = 10Mbps;
}

network GarbageNetwork
{
    @display("bgb=349,240");
    submodules:
        smartphone: Smartphone {
            @display("p=204,69");
        }
        can1: Can {

            parameters:
                hasGarbage = default(false);
                @display("p=253,171");

        }
        can2: Can {
            parameters:
                hasGarbage = default(false);
                @display("p=67,69");
        }
        cloud: Cloud {

            @display("p=74,160");
        }
    connections allowunconnected:
        smartphone.out[0] --> SlowLink --> can1.in[0];
        can1.out --> SlowLink --> smartphone.in[0];

        smartphone.out[1] --> SlowLink --> can2.in[0];
        can2.out --> SlowLink --> smartphone.in[1];

        smartphone.out[2] --> SlowLink --> cloud.in[0];
        cloud.out[0] --> SlowLink --> smartphone.in[2];

        can1.outCloud --> FastLink --> cloud.in[1];
        cloud.out[1] --> FastLink --> can1.in[1];

        can2.outCloud --> FastLink --> cloud.in[2];
        cloud.out[2] --> FastLink --> can2.in[1];
}
