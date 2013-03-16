#ifndef LETTER_HPP_
#define LETTER_HPP_

#include "random.hpp"
#include <cstdlib>

/*
 * Letters tree.
 * This structure is thought to be repeated in an array. A letter x in the array
 * represents p(x|s), where s is the precedent sequence of letters (and so arrays
 * and their specific indexes that have been visited before).
 * The array is terminated like a C-string, with a letter with val = 0.
 */
class letter{

	const static char emptychain_buff = 0;	//Terminators (val == 0) have only one byte allocated, save 8 bytes on 32-bit machine.
	constexpr static letter* emptychain = reinterpret_cast<letter*>(emptychain_buff);

	letter() = delete;

public:

	char val;
	unsigned int occurrences;
	letter* nextletters;

	static letter* newroot(){ return emptychain; }		//p(x)

	static void addoccurrence(letter*& base, char val){
		letter* l = base->find(val);
		if(l) l->occurrences++;
		else{
			int tot = base->letterstotal();
			base = reinterpret_cast<letter*>(std::realloc(tot?base:0, sizeof(letter)*(tot+1)+1));		//if tot==0 then base==emptychain, which must not be reallocated
			base[tot].val = val;
			base[tot].occurrences = 1;
			base[tot].nextletters = emptychain;
			base[tot+1].val = 0;
		}
	}

	int letterstotal(){
		int i = 0;
		for(const letter* cur = this; cur->val; cur++, i++);
		return i;
	}

	letter* find(char val){
		for(letter* cur = this; cur->val; cur++) if(cur->val == val) return cur;
		return nullptr;
	}

	letter* chooserandom(){
		unsigned int o = 0;
		for(const letter* cur = this; cur->val; o+=cur->occurrences, cur++);
		o = random(o);
		letter* cur = this;
		for(; o>=cur->occurrences; o-=cur->occurrences, cur++);
		return cur;
	}

	~letter(){

		if(val && nextletters != emptychain)				//delete next only if I'm not a terminator and I point to a non empty chain
			delete[] nextletters;
	}

}  __attribute__ ((packed));								//Prevent padding done by the compiler, as the main constraint here is memory, not speed.

#endif /* LETTER_HPP_ */
