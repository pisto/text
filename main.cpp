#include <iostream>
#include <stdexcept>
#include <string>
#include <fstream>
#include <unistd.h>
#include <cstring>
#include <algorithm>
#include <cmath>
#include <functional>
#include "utils.hpp"
#include "letter.hpp"
using namespace std;

static bool lowercase = true;
static char* allowedchars;
static size_t prevseqlen = 10;
static u_intg generatedtextlen = 0;
static bool abruptend = false, suddenbegin = false;
static char* file = nullptr;

//Used in the calculation of DS later
static real sum_p_x_s(letter* choice, u_intg needmore, u_intg weigth){
	real tot = 0;
	//if next choice is all the x in p(x|s)
	if(!needmore){
		real totoccs = choice->occurrencestotal();
		for(letter* l = choice; l->val; l++){
			real p = l->occurrences/totoccs;
			tot-=p*log(p);
		}
		tot*=weigth;
	}
	//else go down in the tree further
	else for(letter* l = choice; l->val; l++) tot+=sum_p_x_s(l->nextchoice, needmore-1, l->occurrences);	//weight for p(s)
	return tot;
}

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
	istream& input = finput.is_open()?finput:cin;

	letter* root = letter::newroot();
	u_intg inputlen = 0;

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
			loopai(bufflen) lastletter = letter::addoccurrence(root, lastletter, buffer[prevseqlen-bufflen+i]);
			letter::addoccurrence(root, lastletter, ch);		//add p(ch|seq)
			pushchar(ch);
			inputlen++;
		}
		if(inputlen<prevseqlen+1){
			delete root;
			throw runtime_error("Inputed text is too short to have even a single p(x|s)!");
		}

#define seqsofsize(sz) (inputlen - (sz) + 1)
		real DSarray[prevseqlen+1];
		//DS_m, 1<=m<=prevseqlen + 1, DS_i = hm[i-1]
#define DS(m) (DSarray[(m)-1])
		//calculate DS_m (= h_m)
		for(u_intg m = 1; m<= prevseqlen+1; m++)
			//calc DS by doing sum p(x|s)log(p(x|s)) over x recursively
			DS(m)=(sum_p_x_s(root, m-1, 1)/(m==1?1:seqsofsize(m-1)))/LN2;
		//S_m, 1<=m<=prevseqlen+1
#define blockentropy(m)({\
			real Sm = 0;\
			for(u_intg mi = 1; mi<=m; mi++) Sm+=DS(mi);\
			Sm;\
		})
		//k_m, 1<=m<=prevseqlen+1
#define k(m) (-DSarray[(m)-1]+((m)==1?log(real(root->letterstotal()))/LN2:DSarray[(m)-2]))

		cerr<<"DS = {";
		for(u_intg m = 1; m<=prevseqlen+1; m++){
			if(m!=1) cerr<<", ";
			cerr<<'{'<<m<<", "<<DS(m)<<'}';
		}
		cerr<<"};\nS = {";
		for(u_intg m = 1; m<=prevseqlen+1; m++){
			if(m!=1) cerr<<", ";
			cerr<<'{'<<m<<", "<<blockentropy(m)<<'}';
		}
		cerr<<"};\nk = {";
		for(u_intg m = 1; m<=prevseqlen+1; m++){
			if(m!=1) cerr<<", ";
			cerr<<'{'<<m<<", "<<k(m)<<'}';
		}
		cerr<<"};\n";


		//generate text
		bufflen = 0;
		if(!suddenbegin){		//pretend that ". " has been generated before
			memcpy(buffer+prevseqlen-2, ". ", 2);
			bufflen = 2;
		}
		u_intg written = 0;
		while(generatedtextlen && (written<generatedtextlen || (abruptend?false:buffer[prevseqlen-1]!='.'))){
			letter* lastinseq = nullptr;
			//reconstruct the sequence used so far
			loopai(bufflen) lastinseq = (lastinseq?lastinseq->nextchoice:root)->find(buffer[prevseqlen-bufflen+i]);
			letter* chosen = (lastinseq?lastinseq->nextchoice:root)->chooserandom();	//lastinseq can be null when we begin writing the text
			if(!chosen){
				//this happens if the sequence at the end of the text is unique. Start over with p(x)
				char prev[bufflen+1];
				strncpy(prev, buffer+prevseqlen-bufflen, bufflen);
				cerr<<"Warning: no choice of p(x|s) when s = "<<prev<<'\n';
				chosen = root->chooserandom();
				bufflen = 0;
			}
			cout<<chosen->val;
			pushchar(chosen->val);
			written++;
		}
		cout<<'\n';
		delete root;
	}
	catch(...){ delete root; throw; }

}
