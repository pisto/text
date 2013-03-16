#include <iostream>
#include <stdexcept>
#include <string>
#include <unistd.h>
#include <cstring>
#include <algorithm>
#include <cmath>
#include "letter.hpp"
using namespace std;

static bool lowercase = true;
static char* allowedchars;
static size_t previousseqlen = 10;
static unsigned int generatedtextlen = 0;
static bool abruptend = false, suddenbegin = false;

int main(int argc, char** argv){

	char defaultallowed[] = "qwertyuiopasdfghjklzxcvbnm ?!,.:-'\n";
	allowedchars = defaultallowed;
	char c;
	while((c = getopt(argc, argv, "asuv:p:l:")) != -1) switch(c){
		case 'a': abruptend = true; break;
		case 's': suddenbegin = true; break;
		case 'u': lowercase = false; break;
		case 'v': allowedchars = optarg; break;
		case 'p':
			previousseqlen = atoi(optarg);
			if(!previousseqlen) throw invalid_argument("Invalid previous sequence length "+string(optarg));
			break;
		case 'l':
			generatedtextlen = atoi(optarg);
			if(!generatedtextlen && strcmp("0", optarg)) throw invalid_argument("Invalid generated text length "+string(optarg));
			break;
		default: throw invalid_argument("Unrecognized argument -"+string({char(optopt), 0}));
	}

	size_t allowedlen = strlen(allowedchars);
	sort(allowedchars, allowedchars+allowedlen);
#define ignorechar(ch) (!binary_search(allowedchars, allowedchars+allowedlen, ch))
	if(ignorechar('.')) abruptend = true;
	if(ignorechar('.') || ignorechar(' ') || previousseqlen<2) suddenbegin = true;

	letter* root = letter::newroot();

	//learn
	char buffer[previousseqlen];
	size_t bufflen = 0;
	int ch_;
	while((ch_ = cin.get())!=EOF){
		if(lowercase) ch_ = tolower(ch_);
		char ch = ch_;
		if(ignorechar(ch)) continue;
		letter* lastletter = nullptr;
		//sliding the sequence window creates new occurrences of shorter sequences (sseq) with length 0<l<previousseqlen
		for(size_t i=0; i<bufflen; i++) lastletter = letter::addoccurrence(root, lastletter, buffer[previousseqlen-bufflen+i]);
		letter::addoccurrence(root, lastletter, ch);		//add p(ch|seq)
		size_t copylen = bufflen-(bufflen==previousseqlen);
		memmove(buffer+previousseqlen-copylen-1, buffer+previousseqlen-copylen, copylen);		//push down past characters
		buffer[previousseqlen-1]=ch;
		bufflen = min(bufflen + 1, previousseqlen);
	}

	//TODO print statistics

	//TODO generate text

	delete root;

}
