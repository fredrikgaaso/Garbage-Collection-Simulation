channel Smartphone_Cloud_Link extends ned.DatarateChannel
{
    delay = 5ms;
    datarate = 340Mbps;
}
channel Can_Cloud_Link extends ned.DatarateChannel
{
    delay = 25ms;
    datarate = 100Mbps;
}

channel Smartphone_Can_Link extends ned.DatarateChannel
{
    delay = 100ms;
    datarate = 2.1Mbps;
}	

network GarbageNetwork
{
    @display("bgb=4300,1600");
    @figure[background](type=rectangle; pos=0,0; size=2600,1600; fillColor="#d0d0d0"; lineColor="#b0b0b0"; lineWidth=1; zIndex=-10);
    @figure[outerTop](type=rectangle; pos=450,300; size=1820,3; fillColor="#000000"; lineColor="#000000"; zIndex=-5);
    @figure[outerBottom](type=rectangle; pos=450,1377; size=1820,3; fillColor="#000000"; lineColor="#000000"; zIndex=-5);
    @figure[outerLeft](type=rectangle; pos=450,300; size=3,1080; fillColor="#000000"; lineColor="#000000"; zIndex=-5);
    @figure[innerTop](type=rectangle; pos=620,480; size=1650,3; fillColor="#000000"; lineColor="#000000"; zIndex=-5);
    @figure[innerBottom](type=rectangle; pos=620,1197; size=1650,3; fillColor="#000000"; lineColor="#000000"; zIndex=-5);
    @figure[innerLeft](type=rectangle; pos=620,480; size=3,717; fillColor="#000000"; lineColor="#000000"; zIndex=-5);

    submodules:
        smartphone: Smartphone {

            @display("p=2200,420");
        }
        can1: Can {
            parameters:
                hasGarbage = default(false);
                canId = 1;
                x = 784;
                y = 202;
                @display("p=784,202");
        }
        can2: Can {
            parameters:
                hasGarbage = default(false);
                canId = 2;
                x = 1084;
                y = 996;
                @display("p=1084,996");
        }
        cloud: Cloud {
               	parameters:
         	    x = 2300;
         	    y = 750;
            @display("p=2300,750");
        }
    connections allowunconnected:
        smartphone.out[0] --> Smartphone_Can_Link --> can1.in[0];
        can1.out --> Smartphone_Can_Link --> smartphone.in[0];

        smartphone.out[1] --> Smartphone_Can_Link --> can2.in[0];
        can2.out --> Smartphone_Can_Link --> smartphone.in[1];

        smartphone.out[2] --> Smartphone_Cloud_Link --> cloud.in[0];
        cloud.out[0] --> Smartphone_Cloud_Link --> smartphone.in[2];

        can1.outCloud --> Can_Cloud_Link --> cloud.in[1];
        cloud.out[1] --> Can_Cloud_Link --> can1.in[1];

        can2.outCloud --> Can_Cloud_Link --> cloud.in[2];
        cloud.out[2] --> Can_Cloud_Link --> can2.in[1];
}
