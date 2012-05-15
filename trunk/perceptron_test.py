import math
import copy
from collections import defaultdict
import viterbi
import sys
import re
from string import Template
from time import time

Words = defaultdict(list)
phi = defaultdict(int)
alpha = defaultdict(int)
regExp = defaultdict(str)
possible_tags = []
strings = []
codes = defaultdict(str)
cca1 = defaultdict(tuple)
cca2 = defaultdict(tuple)

cca_length = 0
number_of_sentences = 1

def get_words():
    global possible_tags
    global number_of_sentences
    tags = set([])
    training = open('inputs/eng.train', 'r')
    line = training.readline()
    line = training.readline()
    line = training.readline()
    while line:
        l = line.strip()
        if l:
            vals = l.split(' ')
            tags.add(vals[3])
            if not Words.get(vals[0], False):
                Words[vals[0]] = [0, set([])]
            Words[vals[0]][0] += 1
            Words[vals[0]][1].add(vals[3])
        line = training.readline()
    possible_tags = list(tags)
    training.close()
    test = open('inputs/eng.test{0}'.format(sys.argv[2]), 'r')
    line = test.readline()
    line = test.readline()
    line = test.readline()
    while line:
        l = line.strip()
        if not l:
            number_of_sentences += 1
        line = test.readline()
    test.close()


def get_strings():
    global strings
    string = Template('w_i-2=$w_m2,t=$t')
    strings.append(copy.deepcopy(string))
    string = Template('w_i-1=$w_m1,t=$t')
    strings.append(copy.deepcopy(string))
    string = Template('w_i=$w_i,t=$t')
    strings.append(copy.deepcopy(string))
    string = Template('w_i+1=$w_1,t=$t')
    strings.append(copy.deepcopy(string))
    string = Template('w_i+2=$w_2,t=$t')
    strings.append(copy.deepcopy(string))    
    string = Template('t_-2=$t_2,t_-1=$t_1,t=$t')
    strings.append(copy.deepcopy(string))
    string = Template('t_-1=$t_1,t=$t')
    strings.append(copy.deepcopy(string))
    string = Template('t=$t')
    strings.append(copy.deepcopy(string))
    string = Template('w_i-2=$w_m2,w_i-1=$w_m1,t=$t')
    strings.append(copy.deepcopy(string))
    string = Template('w_i-1=$w_m1,w_i=$w_i,t=$t')
    strings.append(copy.deepcopy(string))
    string = Template('w_i=$w_i,w_i+1=$w_1,t=$t')
    strings.append(copy.deepcopy(string))
    string = Template('w_i+1=$w_1,w_i+2=$w_2,t=$t')
    strings.append(copy.deepcopy(string))
    string = Template('w_i-2=$w_m2,w_i-1=$w_m1,w_i=$w_i,t=$t')
    strings.append(copy.deepcopy(string))
    string = Template('w_i-1=$w_m1,w_i=$w_i,w_i+1=$w_1,t=$t')
    strings.append(copy.deepcopy(string))
    string = Template('w_i+2=$w_2,w_i+1=$w_1,w_i=$w_i,t=$t')
    strings.append(copy.deepcopy(string))
    string = Template('t_-1=$t_1,t=$t')
    strings.append(copy.deepcopy(string))
    string = Template('t_-2=$t_2,t=$t')
    strings.append(copy.deepcopy(string))
    string = Template('POS=$pos,t=$t')
    strings.append(copy.deepcopy(string))
    string = Template('POS=$pos,w_i=$w_i,t=$t')
    strings.append(copy.deepcopy(string))
    string = Template('CHUNK=$chnk,t=$t')
    strings.append(copy.deepcopy(string))
    string = Template('CHUNK=$chnk,w_i=$w_i,t=$t')
    strings.append(copy.deepcopy(string))

def get_regExp():
    firstCaps = re.compile('[A-Z].+')
    AllCaps = re.compile('[A-Z]+$')
    numerals = re.compile('.*\d.*')
    dash = re.compile('.*-.*')
    comma = re.compile('.*,.*')
    period = re.compile('.*\..*')
    endVowel = re.compile('.*[AIOUaiou]$')
    regExp[firstCaps] = 'firstCaps'
    regExp[AllCaps] = 'AllCaps'
    regExp[numerals] = 'Numerals'
    regExp[dash] = 'dash'
    regExp[comma] = 'comma'
    regExp[period] = 'period'
    regExp[endVowel] = 'end_vowel'

def get_codeWords():
    code = open('inputs/paths', 'r')
    line = code.readline()
    l = line.strip()
    while l:
        vals = l.split()
        codes[vals[1]] = vals[0]
        line = code.readline()
        l = line.strip()

def get_alpha():
    s = 'outputs_cca_pos_egw30_rounding_currentOnly/alpha_{0}.txt'.format(sys.argv[1])
    al = open(s, 'r')
    line = al.readline()
    while line:
        l = line.strip()
        if l:
            vals = l.split(' ')
            alpha[int(vals[0])] = int(vals[1])
        line = al.readline()

def get_sentence(data):
    sentence = []
    tags = []
    POS = []
    line = data.readline()
    l = line.strip()
    if not line:
        return 0
    while l:
        vals = l.split(' ')
        word = vals[0]
        sentence.append(word)
        POS.append((vals[1],vals[2]))
        tags.append(vals[3])
        line = data.readline()
        l = line.strip()
    return (copy.deepcopy(sentence), copy.deepcopy(tags), copy.deepcopy(POS))

def get_phi():
    s = 'outputs_cca_pos_egw30_rounding_currentOnly/phi_{0}.txt'.format(sys.argv[1])
    data = open(s, 'r')
    line = data.readline()
    while line:
        l = line.strip()
        vals = l.split(' ')
        phi[vals[0]] = int(vals[1])
        line = data.readline()

def get_cca():
    data = open('inputs/egw20M.words-only.txt', 'r')
    line = data.readline()
    l = line.strip()
    while l:
        vals = l.split()
        word = vals[1]
        numbers = vals[3].split(',')
        cca1[word] = copy.deepcopy(tuple(numbers))
        line = data.readline()
        l = line.strip()
    line = data.readline()
    l = line.strip()
    while l:
        vals = l.split()
        word = vals[1]
        numbers = vals[3].split(',')
        cca2[temp] = copy.deepcopy(tuple(vals))
        line = data.readline()
        l = line.strip()

def evaluate():
    global possible_tags
    global strings
    global cca_length
    get_words()
    get_strings()
    get_alpha()
    get_phi()
    get_regExp()
#    get_codeWords()
    get_cca()
#    cca_length = len(cca1['amended'])
    cca_length = 20
    data = open('inputs/eng.test{0}'.format(sys.argv[2]), 'r')
    s = 'outputs_cca_pos_egw30_rounding_currentOnly/result_{0}_{1}.txt'.format(sys.argv[2], sys.argv[1])
    output = open(s, 'w')
    line = data.readline()
    output.write('{0}\n\n'.format(line.strip()))
    line = data.readline()
    vals = get_sentence(data)
    sentence = vals[0]
    correct_tags = vals[1]
    POS = vals[2]
    count = 0
    time1 = 0.0
    time2 = 0.0
    avg_time = 0.0
    time_val = 0.0
    first = True
    while sentence:
#------------------------
#-------TIME-STATS-------
#------------------------
        count += 1
        time2 = time()
        if not first:
            avg_time = (avg_time*(count-1)+(time2-time1))/count
            time_val = int((avg_time)*(number_of_sentences-count))
        first = False
        progress = open('progress_test.txt', 'w')
        progress.write('Percent complete:\n{0}/{1} = {2}%\n\nTime remaining: \n{3} h {4} min {5} sec'.format(int(count), int(number_of_sentences), float(count*100)/float(number_of_sentences), time_val/3600, (time_val%3600)/60, time_val%60))
        time1 = time2
        progress.close()
#--------------------------
#--------------------------
        tags = viterbi.viterbi(sentence, POS, phi, possible_tags, alpha, strings, Words, regExp, codes, cca1, cca_length)
        for i in range(len(sentence)):
            output.write('{0} {1} {2} {3}\n'.format(sentence[i], POS[i][0], correct_tags[i], tags[i]))
        output.write('\n')
        vals = get_sentence(data)
        sentence = vals[0]
        correct_tags = vals[1]
        POS = vals[2]

evaluate()
