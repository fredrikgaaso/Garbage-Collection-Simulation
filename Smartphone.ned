simple Smartphone
{
    parameters:
        string mode;
        xml mobilityConfig;
        double mobilityUpdateInterval @unit(s) = default(0.1s);
		@display("i=device/pocketpc");

    gates:
        output out[3];
        input in[3];
}
