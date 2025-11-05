message GarbageMessage {
    int id;
    string text;
    bool isAck;
  	double sentTime;
  	double prevHopDelay;
  	bool fromCloud;
}