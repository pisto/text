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

	static const char emptychain_buff;	//Terminators (val == 0) have only one byte allocated, save 8 bytes on 32-bit machine.
	static letter* nochoice;

	letter() = delete;

public:

	char val;
	unsigned int occurrences;
	letter* nextchoice;

	static letter* newroot(){ return nochoice; }		//p(x)

	static letter* addoccurrence(letter*& root, letter* lastletter, char val){
		letter*& choice = reinterpret_cast<letter*&>(lastletter?lastletter->nextchoice:root);
		letter* l = choice->find(val);
		if(l){
			l->occurrences++;
			return l;
		}
		else{
			int tot = choice->letterstotal();
			choice = reinterpret_cast<letter*>(std::realloc(tot?choice:0, sizeof(letter)*(tot+1)+1));		//if tot==0 then base==emptychain, which must not be reallocated
			choice[tot].val = val;
			choice[tot].occurrences = 1;
			choice[tot].nextchoice = nochoice;
			choice[tot+1].val = 0;
			return choice+tot;
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

	void operator delete[](void* mem){
		for(letter* cur = reinterpret_cast<letter*>(mem); cur->val && cur->nextchoice != nochoice; cur++) delete[] cur->nextchoice;
		if(mem != nochoice) free(mem);
	}

	void operator delete(void* mem){ delete[] reinterpret_cast<letter*>(mem); }

}  __attribute__ ((packed));								//Prevent padding done by the compiler, as the main constraint here is memory, not speed.

#endif /* LETTER_HPP_ */
