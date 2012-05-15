import sys
import math
import copy
import re
from string import Template
from collections import defaultdict
from viterbi import viterbi
from time import time


Words = defaultdict(list)
phi = defaultdict(int)
alpha = defaultdict(int)
alpha_average = defaultdict(tuple) #(total sum, example number of last update, value of last update)
possible_tags = []
strings = []
regExp = defaultdict(str)
codes = defaultdict(str)
repeat = defaultdict(list) #repeat[vals] = (# of times correctly tagged in a row, # of skip times remaining)
cca1 = defaultdict(tuple)
cca2 = defaultdict(tuple)

T_DEFAULT = 60
add_factor = 1
number_of_sentences = 0
cca_length = 0

def get_tags():
    global possible_tags
    global number_of_sentences
    tags = set([])
    data = open(sys.argv[1], 'r')
    line = data.readline()
    line = data.readline()
    line = data.readline()
    while line:
        l = line.strip()
        if l:
            vals = l.split(' ')
            tags.add(vals[3])
            if not Words.get(vals[0], False):
                Words[vals[0]] = [0, set([])]
            Words[vals[0]][0] += 1
            Words[vals[0]][1].add(vals[3])
        else:
            number_of_sentences += 1
        line = data.readline()
    possible_tags = list(tags)
    data.close()
    number_of_sentences += 1

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

def get_sentence_and_tags(data):
    sentence = []
    tags = []
    POS = []
    line = data.readline()
    l = line.strip()
    if not line:
        return 0
    while l:
        vals = l.split(' ')
        sentence.append(vals[0])
        tags.append(vals[3])
        POS.append((vals[1],vals[2]))
        line = data.readline()
        l = line.strip()
    return (copy.deepcopy(sentence), copy.deepcopy(tags), copy.deepcopy(POS))

def get_alpha_indices(strings, d, examp_num):
    positions = []
    for s in strings:
        index = phi.get(s.substitute(d), -1)
        if index == -1:
            index = len(phi)
            phi[s.substitute(d)] = index
            alpha[index]
            alpha_average[index] = (0, examp_num, 0)
        positions.append(index)
    return copy.deepcopy(positions)

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

def add_feature(phrase, examp_num):
    index = len(phi)
    phi[phrase] = index
    alpha[index]
    alpha_average[index] = (0, examp_num, 0)
    return index

def get_indices(sent, tag, POS,  examp_num):
    global strings
    result = []
    r = len(sent)
    part_speach = [('*','*'),('*','*')] + POS + [('_STOP','_STOP_'),('_STOP','_STOP_'),('_STOP','_STOP_')]
    tags = ['*', '*'] + tag + ['STOP']
    sentence = ['_*_', '_*_'] + sent + ['_STOP_', '_STOP_', '_STOP_']
    for i in range(r+1):
##########Strings, trigram model############
        d = dict(w_m2 = sentence[i], w_m1 = sentence[i+1], w_i = sentence[i+2], w_1 = sentence[i+3], w_2 = sentence[i+4], t_2 = tags[i], t_1 = tags[i+1], t = tags[i+2], pos = part_speach[i+2][0], chnk = part_speach[i+2][1])
        result += get_alpha_indices(strings, d, examp_num)
##############regular Expressions############
        for  k in range(cca_length):
            if i == r:
                break
            if cca1.get(sentence[i+2], -2) == -2:
                break
            size = .1
            for temp in range(10):
                size = size/10
                cca_val = int(float(cca1[sentence[i+2]][k])/size)
                phrase = 'cca:current,pos={0},size = {1},bucket={2},t={3}'.format(k,size,cca_val,tags[i+2])
                index = phi.get(phrase, -1)
                if index == -1:
                    index = add_feature(phrase, examp_num)
                result.append(index)
#        for k in range(cca_length):
#            if i == r:
#                break
#            if cca1.get(sentence[i+1], -1) == -1:
#                break
#            phrase = 'cca:previous,pos={0},t={1}'.format(k, tags[i+2])
#            index = phi.get(phrase, -1)
#            if index == -1:
#                index = add_feature(phrase, examp_num)
#            result.append(index)
#        for k in range(cca_length):
#            if i == r:
#                break
#            if cca1.get(sentence[i+2], -1) == -1:
#                break
#            phrase = 'cca:current,pos={0},t={1}'.format(k, tags[i+2])
#            index = phi.get(phrase, -1)
#            if index == -1:
#                index = add_feature(phrase, examp_num)
#            result.append(index)
        for j in regExp:
            if i == r:
                break
            if j.match(sentence[i+2]):
                phrase = 're={0},t={1}'.format(regExp[j],tags[i+2])
                index = phi.get(phrase, -1)
                if index == -1:
                    index = add_feature(phrase, examp_num)
                result.append(index)
        if (not i == r) and Words[sentence[i+2]][0] < 5:
            phrase = 'rare=_RARE_,t={0}'.format(tags[i+2])
            index = phi.get(phrase, -1)
            if index == -1:
                index = add_feature(phrase, examp_num)
            result.append(index)
        length = len(sentence[i+2])
        if length > 4 and not i == r: 
            phrase = 'last_four={0},t={1}'.format(sentence[i+2][length-4:length],tags[i+2])
            index = phi.get(phrase, -1)
            if index == -1:
                index = add_feature(phrase, examp_num)
            result.append(index)
        if codes.get(sentence[i+2], False) and not i == r:
            code = codes[sentence[i+2]]
            for c_len in range(2, len(code), 2):
                st = code[:c_len]
                phrase = 'code={0},t={1}'.format(code[:c_len],tags[i+2])
                index = phi.get(phrase, -1)
                if index == -1:
                    index = add_feature(phrase, examp_num)
                result.append(index)
            st = code
            phrase = 'code={0},t={1}'.format(code,tags[i+2])
            index = phi.get(phrase, -1)
            if index == -1:
                index = add_feature(phrase, examp_num)
            result.append(index)    
    return copy.deepcopy(result)

def perceptron():
    global possible_tags
    global strings
    global add_factor
    global cca_length
    get_regExp()
    get_strings()
    get_tags()
#    get_codeWords()
    get_cca()
    cca_length = 50
    for t in range(1,T_DEFAULT+1):
        print '---{0}---'.format(t)
        sys.stdout.flush()
        dont_repeat = True
        data = open(sys.argv[1], 'r')
        data.readline()
        data.readline()
        vals = get_sentence_and_tags(data)
        j = 0
        examp_num = 0
        count = 0.0
        time1 = 0.0
        time2 = 0.0
        avg_time = 0.0
        time_val = 0.0
        first = True
        while vals:
#------------------------
#-------TIME-STATS-------
#------------------------
            count += 1
            time2 = time()
            if not first:
                avg_time = (avg_time*(count-1)+(time2-time1))/count
                time_val = int((avg_time)*(number_of_sentences-count))
            first = False
            progress = open('outputs_cca_pos_egw30_multibucket/progress3.txt', 'w')
            progress.write('Percent complete:\n{0}/{1} = {2}%\n\nTime remaining: \n{3} h {4} min {5} sec'.format(int(count), int(number_of_sentences), float(count*100)/float(number_of_sentences), time_val/3600, (time_val%3600)/60, time_val%60))
            time1 = time2
            progress.close()
#--------------------------
#--------------------------
            examp_num += 1
            sentence = vals[0]
            correct_tags = vals[1]
            POS = vals[2]
            tags = viterbi(sentence, POS, phi, possible_tags, alpha, strings, Words, regExp, codes, cca1, cca_length)
            indices = get_indices(sentence, tags, POS, examp_num)
            if not tags == correct_tags:
                j += 1
                repeat[count] = [0,0]
                dont_repeat = False
                correct_indices = get_indices(sentence, correct_tags, POS, examp_num)
                for i in indices:
                    alpha[i] += -1*add_factor
                for i in correct_indices:
                    alpha[i] += add_factor
            for i in set(indices) | set(correct_indices):
                val1 = alpha_average[i][0]+(examp_num - alpha_average[i][1])*alpha_average[i][2]
                val2 = examp_num
                val3 = alpha[i]
                alpha_average[i] = (val1,val2,val3)
            vals = get_sentence_and_tags(data)
        data.close()
        if dont_repeat:
            print 'SUCCESS!!!'
            break
        print 'number incorrect: {0}'.format(j)
        for i in alpha:
            val1 = alpha_average[i][0]+(examp_num+1 - alpha_average[i][1])*alpha_average[i][2]
            val2 = 1
            val3 = alpha[i]
            alpha_average[i] = (val1,val2,val3)
        if t%10 == 0:
            write_alpha(t)

def write_alpha(t):
    global alpha_average
    string = 'outputs_cca_pos_egw30_multibucket/alpha_{0}.txt'.format(t)
    out = open(string, 'w')
    for i in alpha_average:
        out.write('{0} {1}\n'.format(i, alpha_average[i][0]))
    out.close()
    string = 'outputs_cca_pos_egw30_multibucket/phi_{0}.txt'.format(t)
    out = open(string, 'w')
    for i in phi:
        out.write('{0} {1}\n'.format(i, phi[i]))
    out.close()

perceptron()
