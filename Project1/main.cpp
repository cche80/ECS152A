#include <iostream>
#include <list>
#include <queue>
#include <math.h> //for log function
#include <cstdlib> //for drand48 in linux

// Part of 3.2
// Declare some variables as global
const int MAXBUFFER = 10;

//Set service (mu) and arrival (lambda) rate of the packets
const double ARRIVALRATE = 0.1;
const double SERVICERATE = 0.1;

double globalTime = 0;		// Global time
int bufferLength = 0;		// Number of packets in buffer's queue (includeing the one being transmitted)
int packetsDropped = 0;		// Count of total packets droppedeventList


using namespace std;

///////////////////// Event Class //////////////////////////
class Event {
public:
	Event(){}	// Default Constructor with no event.

	Event(bool inputType, double inputEventTime) {
		type = inputType;
		eventTime = inputEventTime;
	}

	bool getType(void) { return type; }

	void setType(bool inputType) { type = inputType; }

	double getEventTime(void) { return eventTime; }

	void setEventTime(double inputTime) { eventTime = inputTime; }

private:
	bool type; // 1 for arrival event, 0 for departure event.
	double eventTime;
};


///////////////////// GEL Class //////////////////////////
class GEL {
public:
	GEL(void){}
	GEL(Event inputEvent) {} //INCOMPLETE

	void insertEvent(Event &inputEvent) {
		list<Event>::iterator itr;

		for (itr = gel.begin(); itr != gel.end(); itr++) {
			if (inputEvent.getEventTime() <= itr->getEventTime()) { break; }
		}

		gel.insert(itr, inputEvent); // this handles the case of empty gel
	}

	Event getFirstEvent(){ //get the first element
		if (!gel.empty())
			return gel.front();
		else
			cout << "gel empty - can't return front element.";
	}

	void removeFirstEvent() { //remove first element
		if (!gel.empty())
			gel.pop_front();
		else
			cout << "gel empty - can't pop first element.";
	}

private:
	list<Event> gel; //Global Event List
};


///////////////////// Packet Class //////////////////////////
class Packet {
public:
	Packet(double inputTime) { packetServiceTime = inputTime; }

	void setPacketServiceTime(double inputTime) { packetServiceTime = inputTime; }
	double getPacketServiceTime() { return packetServiceTime; }
private:
	double packetServiceTime; //the service time for EACH individual packet, theoretically based on the different packet lengths
};


///////////////////// Buffer Class //////////////////////////
class Buffer { //implemented as FIFO queue
public:
	Buffer(int inputMaxBuffer) { maxBuffer = inputMaxBuffer; }

	bool insertPacket(Packet& inputPacket) {	// pushes to end of queue
		// return 0 (fail) if the packet has to be dropped
		if (buffer.size() >= maxBuffer) { return 0; }

		else {
			buffer.push(inputPacket);
			return 1;
		}
	}

	// PLEASE remember to return! if not the function would return an new initialized Packet
	Packet accessOldestPacket() {
		return buffer.front();
	}

	Packet accessLatestPacket() {
		return buffer.back();
	}

	void removePacket() { //just pops off the first element from queue
		buffer.pop();
	}

	int getBufferSize() { return buffer.size(); }
private:
	queue<Packet> buffer;
	int maxBuffer;
};



double negExpDistTime(double rate) { //Function taken from Project Document
	double u;
	u = drand48(); //drand48 returns a double in the range of [0.0, 1.0)
	return ((-1 / rate)*log(1 - u));
}


// 3.3 - Version 2: pass everything in reference
void processArrivalEvent(GEL& eventList, Buffer& buffer) {
	globalTime = eventList.getFirstEvent().getEventTime();

	double nextArrivalTime = globalTime + negExpDistTime(ARRIVALRATE);

	Packet newPacket(negExpDistTime(SERVICERATE));	// Initialize a new packet

	Event newArrivalEvent(1, nextArrivalTime);		// Initialize a new arrival event
	eventList.insertEvent(newArrivalEvent);

	if (bufferLength == 0) {						// Buffer empty, up for transmission immediately
		double nextDepartureTime = globalTime + newPacket.getPacketServiceTime();
		Event nextDepartureEvent(0, nextDepartureTime);	// Schedule new departure event
		eventList.insertEvent(nextDepartureEvent);
		bufferLength = 1;	// Now there is one being transmitted!
		cout << "Up for Transmission Immediately!" << endl;
		cout << "nextDepartureTime: " << nextDepartureTime << endl;
	}

	else {		// Busy, need to queue up.
		if (buffer.insertPacket(newPacket)) {	// inserPacket() returns 1 -> successful, no packet drop
			bufferLength = buffer.getBufferSize() + 1;	// Update global buffer size, + 1 because now there is one being transmitted

			cout << "Link Busy Transmitting, Pushing the Packet to a queue......" << endl;
			cout << "Packet Service Time pushed onto the buffer: " << buffer.accessLatestPacket().getPacketServiceTime() << endl;
		}

		else {
			packetsDropped++;	// Drop it
			cout << "A Packet was dropped because the buffersize is: " << bufferLength << endl;
		}
	}
}



//3.4 - Processing a Departure Event
void processServiceCompletion(GEL& eventList, Buffer& buffer) {
	globalTime = eventList.getFirstEvent().getEventTime();

	bufferLength = buffer.getBufferSize();

	if (bufferLength > 0) {
		double nextDepartureTime = globalTime + buffer.accessOldestPacket().getPacketServiceTime();

		cout << "Next PacketServiceTime: " << buffer.accessOldestPacket().getPacketServiceTime() << endl;
		cout << "Next DepartureTime: " << nextDepartureTime << endl;

		Event newDepartureEvent(0, nextDepartureTime);
		eventList.insertEvent(newDepartureEvent);
		buffer.removePacket();
	}

	else {
		cout << "Nothing to be transmitted for now. Link is IDLE!" << endl;
	}
}



int main() {
	//3.2 - Initialization
	GEL eventList;
	Buffer buffer(MAXBUFFER);

	//create first arrival event and insert it into the GEL
	Event firstEvent;
	firstEvent.setType(1); //type is arrival
	firstEvent.setEventTime(globalTime + negExpDistTime(ARRIVALRATE));
	//cout<<"time: "<<firstEvent.getEventTime()<<endl;

	eventList.insertEvent(firstEvent);

	//Main Program Loop
	for (int i = 0; i < 100; i++){
		//		Event nextEvent(eventList.getFirstEvent());	// Might not be necessary

		if (eventList.getFirstEvent().getType() == 1){ //if arrival
			cout << "Processing Arrival Event on iteration: " << i << endl;
			processArrivalEvent(eventList, buffer);
		}

		else{
			cout << "Processing Service Completion on iteration: " << i << endl;
			processServiceCompletion(eventList, buffer);
		}

		cout << "Global Time: " << globalTime << endl << endl;
		eventList.removeFirstEvent();
	}

	cout << "Number of Packet Dropped: " << packetsDropped << endl;

	return 0;
}
