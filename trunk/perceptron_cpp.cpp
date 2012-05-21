#include <iostream>
#include <fstream>
#include <cctype>
#include <string>
#include <map>
#include <list>
#include <time.h>
#include <stdlib.h>
#include <stdio.h>
#include <sstream>
#include <math.h>
#include <time.h>

using namespace std;

struct Words_struct{
  int count;
  list<string> tags;
};

struct alpha_avg_struct{
  int total_sum;
  int ex_num_last_update;
  int val_last_update;
};

//key = word, value = number of times each word is seen and the tags associated with word
map<string, Words_struct> Words;

//key = unique string corresponding to a feature
//value = weight of feature
map<string, int> alpha;

//key = unique string corresponding to a feature
//value = average weight of feature
map<string, alpha_avg_struct> alpha_avg;

//list of all tags seen in training data
list<string> possible_tags;

//key = word
//value = brown cluster code
map<string, string> brown_codes;

//key = word
//value = cca row vector corresponding to row
map<string, list<double> > cca1;

//key = word
//value = cca row vector corresponding to row
map<string, list<double> > cca2;

int NUM_ITERATIONS = 50;
int number_of_sentences = 0;
int cca_length = 5;

bool use_heuristic = false; //If a word is seen more than 5 times in training data limit the possible tags to only those seen with the word
bool use_brown = false;
bool use_cca = true;

string training_data_file = "inputs/eng.train";
string brown_codes_file = "inputs/paths";
string cca_file = "inputs/jenny_good.txt";
string out_directory = "no_heuristic/";
string test_directory = "inputs/";

const int  num_test_files = 2;
const string test_files[num_test_files] = {"eng.testa", "eng.testb"};

void init_Words();

void init_brown_codes();

void init_cca();

void get_features_strings(list<string>* features, map<string, string>* local_data);

void get_features_regExp(list<string>* features, map<string, string>* local_data);

void get_features_brown_codes(list<string>* features, map<string, string>* local_data);

void get_features_cca(list<string>* features, map<string, string>* local_data);

void get_features_aux(list<string>* features, map<string, string>* local_data);

void get_features(list<string>* features, map<string, string>* local_data);

void get_all_features(list<string>* sentence, list<string>* tags, list<string>* chnks, list<string>* POS, list<string>* features);

bool get_sentence_and_tags(fstream* data, list<string>* sentence, list<string>* tags, list<string>* chnks, list<string>* POS);

void viterbi(list<string>* sentence, list<string>* chnks, list<string>* POS, list<string>* guess);

void new_feature(string feature, int examp_num);

void perceptron();

void perceptron_test(string test_file, int t);

int main(){
  perceptron();

  return 0;
}

void init_Words(){
  fstream tr_data;
  
  string junk, word, POS, chunk, tag;

  tr_data.open(training_data_file.c_str());

  getline(tr_data, junk);
  getline(tr_data, junk);

  map<string, Words_struct>::iterator Words_it;

  tr_data >> word >> POS >> chunk >> tag;
  while (tr_data.good()){
    Words_it = Words.find(word);
    if (Words_it == Words.end()){
      Words_struct ws;
      ws.count = 0;
      Words[word] = ws;
    }
    
    Words[word].count += 1;
    Words[word].tags.push_front(tag);
    possible_tags.push_front(tag);

    Words[word].tags.sort();
    possible_tags.sort();

    Words[word].tags.unique();
    possible_tags.unique();

    getline(tr_data,junk);
    if (isspace(tr_data.peek())) {
      number_of_sentences += 1;
    }

    tr_data >> word >> POS >> chunk >> tag;
  }
}

void init_brown_codes() {
  fstream codes;
  
  string code, word, junk;
  
  codes.open(brown_codes_file.c_str());
  
  codes >> code >> word;
  while (codes.good()) {
    brown_codes[word] = code;
    getline(codes, junk);
    codes >> code >> word;
  }
}

//the first line of the file must contain the length of the row vectors
void init_cca(){
  fstream cca_data;

  int length;
  
  string word, junk;
  double val;

  cca_data.open(cca_file.c_str());
  
  cca_data >> length >> word;
  while (cca_data.good()) {
    if (not cca1[word].empty()) {
      cout << "bad\n";
    }
    for (int i = 0; i < length; i++) {
      cca_data >> val;
      cca1[word].push_back(val);
    }
    getline(cca_data, junk);
    if (isspace(cca_data.peek())) {
      break;
    }
    cca_data >> word;
  }
  cca_data >> word;
  while (cca_data.good()) {
    for (int i = 0; i < length; i++) {
      cca_data >> val;
      cca2[word].push_back(val);
    }
    cca_data >> word;
  }
}

void get_features_strings(list<string>* features, map<string, string>* local_data){
  string s;
  s = string("w_i-2=") + (*local_data)["w_m2"] + string(",t=") + (*local_data)["t"];
  features->push_front(s);
  s = string("w_i-1=") + (*local_data)["w_m1"] + string(",t=") + (*local_data)["t"];
  features->push_front(s);
  s = string("w_i=") + (*local_data)["w_i"] + string(",t=") + (*local_data)["t"];
  features->push_front(s);
  s = string("w_i+1=") + (*local_data)["w_1"] + string(",t=") + (*local_data)["t"];
  features->push_front(s);
  s = string("w_i+2=") + (*local_data)["w_2"] + string(",t=") + (*local_data)["t"];
  features->push_front(s);
  s = string("t_-2=") + (*local_data)["t_2"] + string(",t_-1=") + (*local_data)["t_1"] + string(",t=") + (*local_data)["t"];
  features->push_front(s);
  s = string("t_-1=") + (*local_data)["t_1"] + string(",t=") + (*local_data)["t"];
  features->push_front(s);
  s = string("t=") + (*local_data)["t"];
  features->push_front(s);
  s = string("w_i-2=") + (*local_data)["w_m2"] + string(",w_i-1=") + (*local_data)["w_m1"] + string(",t=") + (*local_data)["t"];
  features->push_front(s);
  s = string("w_i-1=") + (*local_data)["w_m1"] + string(",w_i=") + (*local_data)["w_i"] + string(",t=") + (*local_data)["t"];
  features->push_front(s);
  s = string("w_i=") + (*local_data)["w_i"] + string(",w_i+1=") + (*local_data)["w_1"] + string(",t=") + (*local_data)["t"];
  features->push_front(s);
  s = string("w_i+1=") + (*local_data)["w_1"] + string(",w_i+2=") + (*local_data)["w_2"] + string(",t=") + (*local_data)["t"];
  features->push_front(s);
  s = string("w_i-2=") + (*local_data)["w_m2"] + string(",w_i-1=") + (*local_data)["w_m1"] + string(",w_i=") + (*local_data)["w_i"] + string(",t=") + (*local_data)["t"];
  features->push_front(s);
  s = string("w_i-1=") + (*local_data)["w_m1"] + string(",w_i=") + (*local_data)["w_i"] + string(",w_i+1=") + (*local_data)["w_1"] + string(",t=") + (*local_data)["t"];
  features->push_front(s);
  s = string("w_i=") + (*local_data)["w_i"] + string(",w_i+1=") + (*local_data)["w_1"] + string(",w_i+2=") + (*local_data)["w_2"] + string(",t=") + (*local_data)["t"];
  features->push_front(s);
  s = string("w_i=") + (*local_data)["w_i"] + string(",w_i+1=") + (*local_data)["w_1"] + string(",w_i+2=") + (*local_data)["w_2"] + string(",t=") + (*local_data)["t"];
  features->push_front(s);
  s = string("t_-1=") + (*local_data)["t_1"] + string(",t=") + (*local_data)["t"];
  features->push_front(s);
  s = string("t_-2=") + (*local_data)["t_2"] + string(",t=") + (*local_data)["t"];
  features->push_front(s);
  s = string("POS=") + (*local_data)["pos"] + string(",t=") + (*local_data)["t"];
  features->push_front(s);
  s = string("POS=") + (*local_data)["pos"] + string(",w_i=") + (*local_data)["w_i"] + string(",t=") + (*local_data)["t"];
  features->push_front(s);
  s = string("CHUNK=") + (*local_data)["chnk"] + string(",t=") + (*local_data)["t"];
  features->push_front(s);
  s = string("CHUNK=") + (*local_data)["chnk"] + string(",w_i=") + (*local_data)["w_i"] + string(",t=") + (*local_data)["t"];
  features->push_front(s);
}

void get_features_regExp(list<string>* features, map<string, string>* local_data){
  string word = (*local_data)["w_i"];
  string s;

  if (isupper(word[0])) {
    s = string("re=firstCaps,t=") + (*local_data)["t"];
    features->push_front(s);
  }
  bool all_upper = true;
  bool dash_test = false;
  bool comma_test = false;
  bool period_test = false;
  bool numeral_test = false;
  for (int i = 0; i < word.length(); i++) {
    if (not isupper(word[i])) {
      all_upper = false;
    }
    if (word[i] == '-') {
      dash_test = true;
    }
    if (word[i] == ',') {
      comma_test = true;
    }
    if (word[i] == '.') {
      period_test = true;
    }
    if (isdigit(word[i])) {
      numeral_test = true;
    }    
  }
  if (all_upper) {
    s = string("re=AllCaps,t=") + (*local_data)["t"];
    features->push_front(s);
  }
  if (dash_test) {
    s = string("re=dash,t=") + (*local_data)["t"];
    features->push_front(s);
  }
  if (comma_test) {
    s = string("re=comma_test,t=") + (*local_data)["t"];
    features->push_front(s);
  }
  if (period_test) {
    s = string("re=period,t=") + (*local_data)["t"];
    features->push_front(s);
  }
  if (numeral_test) {
    s = string("re=numerals,t=") + (*local_data)["t"];
    features->push_front(s);
  }
  if (word[word.length()-1] == 'A' || word[word.length()-1] == 'E' || word[word.length()-1] == 'I' || word[word.length()-1] == 'O' || word[word.length()-1] == 'U' || word[word.length()-1] == 'a' || word[word.length()-1] == 'e' || word[word.length()-1] == 'i' || word[word.length()-1] == 'o' || word[word.length()-1] == 'u') {
    s = string("re=end_vowel,t=") + (*local_data)["t"];
    features->push_front(s);
  }
}

void get_features_brown_codes(list<string>* features, map<string, string>* local_data){
  map<string, string>::iterator brown_codes_it;
  string word = (*local_data)["w_i"];
  string s;

  brown_codes_it = brown_codes.find(word);
  if (brown_codes_it == brown_codes.end()) {
    return;
  }
  
  string code = brown_codes[word];

  s = string("code=") + code + string(",t=") + (*local_data)["t"];
  features->push_front(s);

  for (int i = 2; i < code.length(); i += 2) {
    string subcode = code.substr(0,i);
    s = string("code=") + subcode + string(",t=") + (*local_data)["t"];
    features->push_front(s);
  }
}

void get_features_cca(list<string>* features, map<string, string>* local_data) {
  map<string, list<double> >::iterator cca_it;
  string word = (*local_data)["w_i"];
  string s = "";

  cca_it = cca1.find(word);
  if (cca_it == cca1.end()) {
    return;
  }

  list<double>::iterator vect_it = cca1[word].begin();

  for (int i = 0; i < cca_length; i++) {
    double size = .0001;
    for (int j = 0; j < 3; j++) {
      stringstream ss;
      size = size/10.0;
      int bucket = static_cast<int>((*vect_it)/size);
      ss << s << "cca:current,position=" << i << ",size=" << size << ",bucket=" << bucket << ",t=" << (*local_data)["t"];
      features->push_front(ss.str());
    }
    vect_it++;
  }
}

void get_features_aux(list<string>* features, map<string, string>* local_data) {
  map<string, Words_struct >::iterator Words_it;
  string word = (*local_data)["w_i"];
  string s = "";

  Words_it = Words.find(word);
  if (Words_it != Words.end() && Words_it->second.count < 5) {
    string phrase;
    phrase = string("rare:t=") + (*local_data)["t"];
    features->push_front(phrase);
  }
  if (word.size() > 4) {
    string phrase;
    phrase = string("last_four=") + word.substr(word.size()-5,4) + string(",t=") + (*local_data)["t"];
    features->push_front(phrase);
  }
}

void get_features(list<string>* features, map<string, string>* local_data){
  get_features_strings(features, local_data);
  if ((*local_data)["w_i"] == "_STOP_") {
    return;
  }
  get_features_regExp(features, local_data);
  get_features_aux(features, local_data);
  if (use_brown) {
    get_features_brown_codes(features, local_data);
  }
  if (use_cca) {
    get_features_cca(features, local_data);
  }
}

void get_all_features(list<string>* sentence, list<string>* tags, list<string>* chnks, list<string>* POS, list<string>* features) {
  features->clear();

  int len = sentence->size();

  string sent_temp[len+5];
  string tags_temp[len+5];
  string chnks_temp[len+5];
  string pos_temp[len+5];

  sent_temp[0] = "_*_";
  sent_temp[1] = "_*_";
  tags_temp[0] = "_*_";
  tags_temp[1] = "_*_";
  chnks_temp[0] = "_*_";
  chnks_temp[1] = "_*_";
  pos_temp[0] = "_*_";
  pos_temp[1] = "_*_";

  sent_temp[len+2] = "_STOP_";
  sent_temp[len+3] = "_STOP_";
  sent_temp[len+4] = "_STOP_";
  tags_temp[len+2] = "_STOP_";
  tags_temp[len+3] = "_STOP_";
  tags_temp[len+4] = "_STOP_";
  chnks_temp[len+2] = "_STOP_";
  chnks_temp[len+3] = "_STOP_";
  chnks_temp[len+4] = "_STOP_";
  pos_temp[len+2] = "_STOP_";
  pos_temp[len+3] = "_STOP_";
  pos_temp[len+4] = "_STOP_";

  list<string>::iterator sent_it = sentence->begin();
  list<string>::iterator tags_it = tags->begin();
  list<string>::iterator chnks_it = chnks->begin();
  list<string>::iterator pos_it = POS->begin();

  for (int i = 2; i < len+2; i++) {
    sent_temp[i] = *sent_it;
    tags_temp[i] = *tags_it;
    chnks_temp[i] = *chnks_it;
    pos_temp[i] = *pos_it;

    sent_it++;
    tags_it++;
    chnks_it++;
    pos_it++;
  }
  
  for (int i = 2; i < len+3; i++) {
    map<string, string> local_data;
    local_data.clear();

    local_data["w_m2"] = sent_temp[i-2];
    local_data["w_m1"] = sent_temp[i-1];
    local_data["w_i"] = sent_temp[i];
    local_data["w_1"] = sent_temp[i+1];
    local_data["w_2"] = sent_temp[i+2];
    local_data["t_2"] = tags_temp[i-2];
    local_data["t_1"] = tags_temp[i-1];
    local_data["t"] = tags_temp[i];
    local_data["pos"] = pos_temp[i];
    local_data["chnk"] = chnks_temp[i];

    get_features(features, &local_data);
  }
}

bool get_sentence_and_tags(fstream* data, list<string>* sentence, list<string>* tags, list<string>* chnks, list<string>* POS) {
  if ((not sentence->empty()) || (not chnks->empty()) || (not tags->empty()) || (not POS->empty())) {
    cout << "GET_SENTENCE_AND_TAGS ERROR!!\n";
  }
  
  string junk, word, POS_val, chunk_val, tag_val;

  bool result = false;

  (*data) >> word >> POS_val >> chunk_val >> tag_val;
  if (word == "-DOCSTART-") {
    (*data) >> word >> POS_val >> chunk_val >> tag_val;
  }
  while (data->good()) {
    result = true;
    sentence->push_back(word);
    POS->push_back(POS_val);
    chnks->push_back(chunk_val);
    tags->push_back(tag_val);
    getline((*data), junk);
    
    if (isspace(data->peek())) {
      break;
    }
    (*data) >> word >> POS_val >> chunk_val >> tag_val;
  }
  return result;
}

void viterbi(list<string>* sentence, list<string>* chnks, list<string>* POS, list<string>* guess) {
  guess->clear();

  map<string, double> pi;
  map<string, string> bp;

  int len = sentence->size();

  string sent_temp[len+5];
  string chnks_temp[len+5];
  string pos_temp[len+5];

  pi.clear();
  bp.clear();

  pi["0;*;*"] = 1.0;

  sent_temp[0] = "_*_";
  sent_temp[1] = "_*_";
  chnks_temp[0] = "_*_";
  chnks_temp[1] = "_*_";
  pos_temp[0] = "_*_";
  pos_temp[1] = "_*_";

  sent_temp[len+2] = "_STOP_";
  sent_temp[len+3] = "_STOP_";
  sent_temp[len+4] = "_STOP_";
  chnks_temp[len+2] = "_STOP_";
  chnks_temp[len+3] = "_STOP_";
  chnks_temp[len+4] = "_STOP_";
  pos_temp[len+2] = "_STOP_";
  pos_temp[len+3] = "_STOP_";
  pos_temp[len+4] = "_STOP_";

  list<string>::iterator sent_it = sentence->begin();
  list<string>::iterator chnks_it = chnks->begin();
  list<string>::iterator pos_it = POS->begin();

  for (int i = 2; i < len+2; i++) {
    sent_temp[i] = *sent_it;
    chnks_temp[i] = *chnks_it;
    pos_temp[i] = *pos_it;

    sent_it++;
    chnks_it++;
    pos_it++;
  }
  for (int k = 2; k < len+2; k++) {
    list<string> t1 = possible_tags;
    list<string> t2 = possible_tags;
    list<string> t3 = possible_tags;

    if (k == 2) {
      t1.clear();
      t2.clear();
      
      t1.push_front("*");
      t2.push_front("*");
    }
    else if (k == 3) {
      t1.clear();
      t1.push_front("*");
    }

    if (use_heuristic && (Words.find(sent_temp[k]) != Words.end()) && Words[sent_temp[k]].count > 5) {
      t3.clear();
      t3 = Words[sent_temp[k]].tags;
    }

    list<string>::iterator t1_it;
    list<string>::iterator t2_it;
    list<string>::iterator t3_it;
    for (t1_it = t1.begin(); t1_it != t1.end(); t1_it++) {
      for (t2_it = t2.begin(); t2_it != t2.end(); t2_it++) {
	for (t3_it = t3.begin(); t3_it != t3.end(); t3_it++) {
	  string pi_prev_s = "";
	  string pi_cur_s = "";
	  stringstream pi_prev_ss;
	  stringstream pi_cur_ss;
	  
	  map<string, string> local_data;
	  list<string> features;
	  list<string>::iterator feat_it;
	  double val = 0.0;

	  pi_prev_ss << pi_prev_s << k-2 << ";" << *t1_it << ";" << *t2_it;

	  if (pi.find(pi_prev_ss.str()) == pi.end()) {
	    break;
	  }
	  else {
	    val = pi[pi_prev_ss.str()];
	  }

	  local_data.clear();
	  features.clear();
	  
	  local_data["w_m2"] = sent_temp[k-2];
	  local_data["w_m1"] = sent_temp[k-1];
	  local_data["w_i"] = sent_temp[k];
	  local_data["w_1"] = sent_temp[k+1];
	  local_data["w_2"] = sent_temp[k+2];
	  local_data["t_2"] = *t1_it;
	  local_data["t_1"] = *t2_it;
	  local_data["t"] = *t3_it;
	  local_data["pos"] = pos_temp[k];
	  local_data["chnk"] = chnks_temp[k];

	  get_features(&features, &local_data);

	  for (feat_it = features.begin(); feat_it != features.end(); feat_it++) {
	    if (alpha.find(*feat_it) != alpha.end()) {
	      val += alpha[*feat_it];
	    }
	  }
	  pi_cur_ss << pi_cur_s << k-1 << ";" << *t2_it << ";" << *t3_it;

	  if ((pi.find(pi_cur_ss.str()) == pi.end()) || val > pi[pi_cur_ss.str()]) {
	    pi[pi_cur_ss.str()] = val;
	    bp[pi_cur_ss.str()] = *t1_it;
	  }
	}
      }
    }
  }
  string result_tags[len];
  double result_val;
  bool got_first = false;
  list<string> t2 = possible_tags;
  if (len == 1) {
    t2.clear();
    t2.push_front("*");
  }
  
  list<string>::iterator t1_it;
  list<string>::iterator t2_it;
  for (t2_it = t2.begin(); t2_it != t2.end(); t2_it++) {
    for (t1_it = possible_tags.begin(); t1_it != possible_tags.end(); t1_it++) {
      string pi_s = "";
      stringstream pi_ss;

      map<string, string> local_data;
      list<string> features;
      list<string>::iterator feat_it;
      double val = 0.0;

      pi_ss << pi_s << len << ";" << *t2_it << ";" << *t1_it;
      
      if (pi.find(pi_ss.str()) == pi.end()) {
	continue;
      }
      else {
	val = pi[pi_ss.str()];
      }

      local_data["w_m2"] = sent_temp[len];
      local_data["w_m1"] = sent_temp[len+1];
      local_data["w_i"] = sent_temp[len+2];
      local_data["w_1"] = sent_temp[len+3];
      local_data["w_2"] = sent_temp[len+4];
      local_data["t_2"] = *t2_it;
      local_data["t_1"] = *t1_it;
      local_data["t"] = "_STOP_";
      local_data["pos"] = pos_temp[len+2];
      local_data["chnk"] = chnks_temp[len+2];
      
      features.clear();

      get_features(&features, &local_data);

      for (feat_it = features.begin(); feat_it != features.end(); feat_it++) {
	if (alpha.find(*feat_it) != alpha.end()) {
	  val += alpha[*feat_it];
	}
      }

      if ((val > result_val) || (not got_first)) {
	got_first = true;
	result_tags[len-1] = *t1_it;
	if (len > 1) {
	  result_tags[len-2] = *t2_it;
	}
	result_val = val;
      }
    }
  }
  for (int k = len-2; k > 0; k--) {
    string pi_s = "";
    stringstream pi_ss;
    
    pi_ss << pi_s << k+2 << ";" << result_tags[k] << ";" << result_tags[k+1];
    result_tags[k-1] = bp[pi_ss.str()];
    if (result_tags[0] == "*") {
      cout << "BAD2\n";
    }
  }
  
  for (int i = 0; i < len; i++) {
    guess->push_back(result_tags[i]);
  }
}

void new_feature(string feature, int examp_num) {
  alpha[feature] = 0;
  alpha_avg_struct aas;
  aas.total_sum = 0;
  aas.ex_num_last_update = examp_num;
  aas.val_last_update = 0;
  alpha_avg[feature] = aas;
}

void perceptron() {
  init_Words();
  if (use_brown) {
    init_brown_codes();
  }
  if (use_cca) {
    init_cca();
  }

  for (int t = 1; t < NUM_ITERATIONS + 1; t++) {
    bool dont_repeat = true;
    bool retrieved_data;
    fstream data;
    list<string> sentence, tags, chnks, POS;
    
    int j = 0;
    int examp_num = 0;
    double count = 0.0;
    time_t time1 = 0.0;
    time_t time2 = 0.0;
    double avg_time = 0.0;
    time_t start_time = time(NULL);
    double time_val = 0;
    bool first = true;

    cout << "---" << t << "---\n" << flush;

    data.open(training_data_file.c_str());

    sentence.clear();
    tags.clear();
    chnks.clear();
    POS.clear();
    
    retrieved_data = get_sentence_and_tags(&data, &sentence, &tags, &chnks, &POS);

    while (retrieved_data) {
      //--------------------------------
      //------------TIME-STATS----------
      //--------------------------------      
      ofstream progress;
      string file;

      count += 1;
      time2 = time(NULL);
      if (not first) {
	avg_time = static_cast<double>((time2-start_time))/static_cast<double>(count);
	time_val = avg_time * (number_of_sentences - count);
      }
      first = false;

      file = out_directory + string("progress.txt");
      
      progress.open(file.c_str());

      progress << "Percent complete:\n";
      progress << "----------------\n\n";
      progress << count << "/" << number_of_sentences << " = " << static_cast<double>(count*100)/static_cast<double>(number_of_sentences) << "%\n\n\n";
      progress << "Time remaining:\n";
      progress << "--------------\n\n";
      progress << static_cast<int>(time_val/3600) << " h " << (static_cast<int>(time_val)%3600)/60 << " m " << static_cast<int>(time_val)%60 << " s\n";
      
      time1 = time2;
      progress.close();
      //--------------------------------
      //--------------------------------

      examp_num += 1;

      list<string> guess;

      viterbi(&sentence, &chnks, &POS, &guess);

      if (guess != tags) {
	j += 1;
	dont_repeat = false;

	list<string> features_correct;
	list<string> features_guess;
	list<string>::iterator feat_it;

	get_all_features(&sentence, &tags, &chnks, &POS, &features_correct);
	get_all_features(&sentence, &guess, &chnks, &POS, &features_guess);	

	for (feat_it = features_correct.begin(); feat_it != features_correct.end(); feat_it++) {
	  if (alpha.find(*feat_it) == alpha.end()) {
	    new_feature(*feat_it, examp_num);
	  }
	  alpha[*feat_it] += 1;
	}
	for (feat_it = features_guess.begin(); feat_it != features_guess.end(); feat_it++) {
	  if (alpha.find(*feat_it) == alpha.end()) {
	    new_feature(*feat_it, examp_num);
	  }
	  alpha[*feat_it] += -1;
	}
	
	features_correct.sort();
	features_guess.sort();
	features_correct.merge(features_guess);
	features_correct.unique();

	for (feat_it = features_correct.begin(); feat_it != features_correct.end(); feat_it++) {
	  int val1 = alpha_avg[*feat_it].total_sum + (examp_num - alpha_avg[*feat_it].ex_num_last_update)*alpha_avg[*feat_it].val_last_update;
	  int val2 = examp_num;
	  int val3 = alpha[*feat_it];

	  alpha_avg[*feat_it].total_sum = val1;
	  alpha_avg[*feat_it].ex_num_last_update = val2;
	  alpha_avg[*feat_it].val_last_update = val3;	  
	}
      }

      sentence.clear();
      tags.clear();
      chnks.clear();
      POS.clear();
      
      retrieved_data = get_sentence_and_tags(&data, &sentence, &tags, &chnks, &POS);      
    }
    data.close();
    if (dont_repeat) {
      cout << "SUCCESS!!!\n" << flush;
      return;
    }
    double temp = static_cast<double>(time(NULL)-start_time);
    cout << "Total Time for Iteration: " << static_cast<int>(temp/3600) << " h " << (static_cast<int>(temp)%3600)/60 << " m " << static_cast<int>(temp)%60 << " s\n";
    cout << "Number Incorrect: " << j << endl << flush;

    map<string, int>::iterator alpha_it;
    
    for (alpha_it = alpha.begin(); alpha_it != alpha.end(); alpha_it++) {
      int val1 = alpha_avg[alpha_it->first].total_sum + (examp_num - alpha_avg[alpha_it->first].ex_num_last_update)*alpha_avg[alpha_it->first].val_last_update;
      int val2 = examp_num;
      int val3 = alpha[alpha_it->first];

      alpha_avg[alpha_it->first].total_sum = val1;
      alpha_avg[alpha_it->first].ex_num_last_update = val2;
      alpha_avg[alpha_it->first].val_last_update = val3;
    }
    if (t%10 == 0) {
      for (int i = 0; i < num_test_files; i++) {
	perceptron_test(test_files[i], t);
      }
    }
  }
}

void perceptron_test(string test_file, int t) {
  bool retrieved_data;
  fstream test_data;
  ofstream out;
  list<string> sentence, tags, chnks, POS;
  
  double count = 0.0;
  time_t time1 = 0.0;
  time_t time2 = 0.0;
  double avg_time = 0.0;
  time_t start_time = time(NULL);
  double time_val = 0;
  bool first = true;

  stringstream ss;
  string s = test_directory + test_file;
  test_data.open(s.c_str());
  ss << out_directory << "result_" << t << "_" << test_file;
  out.open((ss.str()).c_str());

  sentence.clear();
  tags.clear();
  chnks.clear();
  POS.clear();
    
  retrieved_data = get_sentence_and_tags(&test_data, &sentence, &tags, &chnks, &POS);

  while (retrieved_data) {
    //--------------------------------
    //------------TIME-STATS----------
    //--------------------------------      
    ofstream progress;
    string file;
    
    count += 1;
    time2 = time(NULL);
    if (not first) {
      avg_time = static_cast<double>((time2-start_time))/static_cast<double>(count);
      time_val = avg_time * (number_of_sentences - count);
    }
    first = false;
    
    file = out_directory + string("progress.txt");
    
    progress.open(file.c_str());
    
    progress << "Percent complete:\n";
    progress << "----------------\n\n";
    progress << count << "/" << number_of_sentences << " = " << static_cast<double>(count*100)/static_cast<double>(number_of_sentences) << "%\n\n\n";
    progress << "Time remaining:\n";
    progress << "--------------\n\n";
    progress << static_cast<int>(time_val/3600) << " h " << (static_cast<int>(time_val)%3600)/60 << " m " << static_cast<int>(time_val)%60 << " s\n";
    
    time1 = time2;
    progress.close();
    //--------------------------------
    //--------------------------------
    
    list<string> guess;

    viterbi(&sentence, &chnks, &POS, &guess);
    
    list<string>::iterator sent_it;
    list<string>::iterator pos_it;
    list<string>::iterator tags_it;
    list<string>::iterator guess_it;
    
    pos_it = POS.begin();
    tags_it = tags.begin();
    guess_it = guess.begin();

    for (sent_it = sentence.begin(); sent_it != sentence.end(); sent_it++) {
      out << *sent_it << " " << *pos_it << " " << *tags_it << " " << *guess_it << "\n";
      pos_it++;
      tags_it++;
      guess_it++;
    }
    
    out << "\n";
    
    sentence.clear();
    tags.clear();
    chnks.clear();
    POS.clear();
    
    retrieved_data = get_sentence_and_tags(&test_data, &sentence, &tags, &chnks, &POS);      
  }      
  test_data.close();
  double temp = static_cast<double>(time(NULL)-start_time);
  cout <<  "Total Time for testing " << test_file << ": " << static_cast<int>(temp/3600) << " h " << (static_cast<int>(temp)%3600)/60 << " m " << static_cast<int>(temp)%60 << " s\n" << flush;
}

























