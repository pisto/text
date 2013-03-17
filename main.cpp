#include <iostream>
#include <stdexcept>
#include <string>
#include <fstream>
#include <unistd.h>
#include <cstring>
#include <algorithm>
#include <cmath>
#include "letter.hpp"
using namespace std;

static bool lowercase = true;
static char* allowedchars;
static size_t prevseqlen = 10;
static unsigned int generatedtextlen = 0;
static bool abruptend = false, suddenbegin = false;
static char* file = nullptr;

int main(int argc, char** argv){

	//parse args
	char defaultallowed[] = "qwertyuiopasdfghjklzxcvbnm ?!,.:-'";
	allowedchars = defaultallowed;
	char c;
	while((c = getopt(argc, argv, "asuv:p:l:i:")) != -1) switch(c){
	case 'a': abruptend = true; break;
	case 's': suddenbegin = true; break;
	case 'u': lowercase = false; break;
	case 'v': allowedchars = optarg; break;
	case 'p':
		prevseqlen = atoi(optarg);
		if(!prevseqlen) throw invalid_argument("Invalid previous sequence length "+string(optarg));
		break;
	case 'l':
		generatedtextlen = atoi(optarg);
		if(!generatedtextlen && strcmp("0", optarg)) throw invalid_argument("Invalid generated text length "+string(optarg));
		break;
	case 'i': file = optarg; break;
	default: throw invalid_argument("Unrecognized argument -"+string({char(optopt), 0}));
	}

	//init
	size_t allowedlen = strlen(allowedchars);
	sort(allowedchars, allowedchars+allowedlen);
#define ignorechar(ch) (!binary_search(allowedchars, allowedchars+allowedlen, ch))
	if(ignorechar('.')) abruptend = true;
	if(ignorechar('.') || ignorechar(' ') || prevseqlen<2) suddenbegin = true;

	ifstream finput;
	if(file){
		finput.open(file);
		if(!finput) throw invalid_argument("Cannot open file "+string(file));
	}
	istream& input = finput?finput:cin;

	letter* root = letter::newroot();

	try{
		//learn
		char buffer[prevseqlen];
		size_t bufflen = 0;
		//push a character on top of buffer, shift down previos characters
#define pushchar(ch) ({\
		size_t copylen = bufflen-(bufflen==prevseqlen);\
		memmove(buffer+prevseqlen-copylen-1, buffer+prevseqlen-copylen, copylen);\
		buffer[prevseqlen-1]=(ch);\
		bufflen = min(bufflen + 1, prevseqlen);\
		})

		int ch_;
		while((ch_ = input.get())!=EOF){
			if(lowercase) ch_ = tolower(ch_);
			char ch = ch_;
			if(ignorechar(ch)) continue;
			letter* lastletter = nullptr;
			//sliding the sequence window creates new occurrences of shorter sequences (sseq) with length 0<l<previousseqlen
			for(size_t i=0; i<bufflen; i++) lastletter = letter::addoccurrence(root, lastletter, buffer[prevseqlen-bufflen+i]);
			letter::addoccurrence(root, lastletter, ch);		//add p(ch|seq)
			pushchar(ch);
		}
		if(!root->letterstotal()){
			delete root;
			throw runtime_error("No text inputed!");
		}

		//TODO print statistics

		//generate text
		bufflen = 0;
		if(!suddenbegin){		//pretend that ". " has been generated before
			memcpy(buffer+prevseqlen-2, ". ", 2);
			bufflen = 2;
		}
		unsigned int written = 0;
		while(generatedtextlen && (written<generatedtextlen || (abruptend?false:buffer[prevseqlen-1]!='.'))){
			letter* lastinseq = nullptr;
			//reconstruct the sequence used so far
			for(size_t i=0; i<bufflen; i++) lastinseq = (lastinseq?lastinseq->nextchoice:root)->find(buffer[prevseqlen-bufflen+i]);
			letter* choice = lastinseq?lastinseq->nextchoice:root;	//lastinseq can be null when we begin writing the text
			if(!choice->letterstotal()){
				choice = root;			//this happens if the sequence at the end of the text is unique. Start over with p(x)
				char prev[bufflen+1];
				strncpy(prev, buffer+prevseqlen-bufflen, bufflen);
				cerr<<"Warning: no choice of p(x|s) when s = "<<prev<<'\n';
			}
			letter* chosen = choice->chooserandom();
			cout<<chosen->val;
			pushchar(chosen->val);
			written++;
		}
		cout<<'\n';
		delete root;
	}
	catch(...){ delete root; throw; }

}
