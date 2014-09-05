#ifndef LETTER_HPP_
#define LETTER_HPP_

#include "utils.hpp"
#include <cstdlib>
#include <random>

/*
 * Letters tree.
 * This structure is thought to be repeated in an array. A letter x in the array
 * represents p(x|s), where s is the precedent sequence of letters (and so arrays
 * and their specific indexes that have been visited before).
 * The array is terminated like a C-string, with a letter with val = 0.
 */
class letter{

	letter(): val(0) {}

public:

	char val;
	u_intg occurrences;
	letter* nextchoice;

	static letter* empty(){
		static letter nochoice;
		return &nochoice;
	}

	static letter* addoccurrence(letter*& root, letter* lastletter, char val){
		letter*& choice = reinterpret_cast<letter*&>(lastletter ? lastletter->nextchoice : root);
		letter* l = choice->find(val);
		if(l){
			l->occurrences++;
			return l;
		}
		else{
			size_t tot = choice->letterstotal();
			choice = reinterpret_cast<letter*>(std::realloc(choice != empty() ? choice : 0, sizeof(letter) * (tot + 1) + 1));
			choice[tot].val = val;
			choice[tot].occurrences = 1;
			choice[tot].nextchoice = empty();
			choice[tot + 1].val = 0;
			return choice + tot;
		}
	}

	u_intg letterstotal(){
		u_intg i = 0;
		for(letter* cur = this; cur->val; cur++, i++);
		return i;
	}

	u_intg occurrencestotal(){
		u_intg tot = 0;
		for(letter* cur = this; cur->val; tot += cur->occurrences, cur++);
		return tot;
	}

	letter* find(char val){
		for(letter* cur = this; cur->val; cur++) if(cur->val == val) return cur;
		return 0;
	}

	letter* chooserandom(){
		u_intg o = occurrencestotal();
		if(!o) return 0;
		static std::mt19937 generator((std::random_device())());
		o = std::uniform_int_distribution<u_intg>(0, o - 1)(generator);
		letter* cur = this;
		while(o >= cur->occurrences) o -= cur->occurrences, cur++;
		return cur;
	}

	void operator delete(void* mem){
		for(letter* cur = reinterpret_cast<letter*>(mem); cur->val; cur++)
			if(cur->nextchoice != empty()) delete cur->nextchoice;
		if(mem != empty()) free(mem);
	}

}  __attribute__ ((packed));								//Prevent padding done by the compiler, as the main constraint here is memory, not speed.

#endif /* LETTER_HPP_ */
