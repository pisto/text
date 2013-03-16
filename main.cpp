#include <cstdlib>
#include <iostream>
#include <fstream>
using namespace std;

ifstream source("/dev/urandom");

unsigned int random(uint modulo){							//just doing random%modulo is not uniform.
	unsigned int max = (0x100000000ULL/modulo)*modulo;		//round to integer number of modulo multiples
	unsigned int r=0xFFFFFFFFU;
	if(!max){												//if modulo is a power of 2 max is 0
		source.read((char*)&r, 4);
		return r%modulo;
	}
	while(r>=max) source.read((char*)&r, 4);				//otherwise it's non-zero, poll rng to get a random number in the range
	return r%modulo;
}

/*
 * Letters tree.
 * This structure is thought to be repeated in an array. A letter x in the array
 * represents p(x|s), where s is the precedent sequence of letters (and so arrays
 * that have been visited before).
 * The array is terminated like a C-string, with a letter with val = 0.
 */
class letter{

	constexpr const static char emptychain_buff = 0;	//Terminators (val == 0) have only one byte allocated, save 8 bytes on 32-bit machine.

public:

	letter() = delete;

	char val;
	unsigned int occurrences;
	letter* nextletters;

	constexpr const static letter* emptychain = (const letter*)emptychain_buff;
	static letter* root;		//p(x)

	static void addoccurrence(letter*& base, char val){
		letter* l = base->find(val);
		if(l) l->occurrences++;
		else{
			int tot = base->letterstotal();
			base = (letter*)realloc(tot?base:0, sizeof(letter)*(tot+1)+1);
			base[tot].val = val;
			base[tot].occurrences = 1;
			base[tot].nextletters = (letter*)emptychain;
			base[tot+1].val = 0;
		}
	}

	int letterstotal(){
		int i = 0;
		for(letter* cur = this; cur->val; cur++, i++);
		return i;
	}

	letter* find(char val){
		for(letter* cur = this; cur->val; cur++) if(cur->val == val) return cur;
		return 0;
	}

	letter* chooserandom(){
		unsigned int totaloccurrences = 0;
		for(letter* cur = this; cur->val; totaloccurrences+=cur->occurrences, cur++);
		unsigned int r = random(totaloccurrences);
		letter* cur = this;
		for(; r>=cur->occurrences; r-=cur->occurrences, cur++);
		return cur;
	}

	~letter(){
		if(val) delete[] nextletters;
	}

}  __attribute__ ((packed));

letter* letter::root = (letter*)letter::emptychain;

int main(){

	//TODO

	delete letter::root;
}
