#include "random.hpp"
#include <fstream>
using namespace std;

ifstream source("/dev/urandom");

unsigned int random(unsigned int modulo){							//just doing random%modulo is not uniform.
	unsigned int max = (0x100000000ULL/modulo)*modulo;		//round to integer number of modulo multiples
	unsigned int r=0xFFFFFFFFU;
	if(!max){												//if modulo is a power of 2 max is 0
		source.read(reinterpret_cast<char*>(&r), 4);
		return r%modulo;
	}
	while(r>=max) source.read(reinterpret_cast<char*>(&r), 4);				//otherwise it's non-zero, poll rng to get a random number in the range
	return r%modulo;
}
