simple Can
{
    parameters:
        bool hasGarbage;
        string mode;
        int canId;
        double x;
        double y;
         @display("i=block/bucket");
    gates:
        input  in[2];
        output out;
        output outCloud;
}
