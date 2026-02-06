#include "SentinelSystem.h"

int main() {
	SentinelSystem main_gate(0);
	//SentinelSystem backdoor(1);

	main_gate.start();
	//backdoor.start();
	return 0;
}