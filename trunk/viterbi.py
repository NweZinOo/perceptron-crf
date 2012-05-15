import math
import copy
from string import Template
from collections import defaultdict

pi = defaultdict(int)
bp = defaultdict(tuple)

beta = 100

def get_alpha_indices(strings, phi, d, Words, regExp, codes, cca1, cca_length):
    positions = []
    for s in strings:
        index = phi.get(s.substitute(d), -1)
        if index == -1:
            continue
        positions.append((index,1))
    for k in range(cca_length):
        if cca1.get(d['w_i'],-1) == -1:
            break
        size = .1
        for temp in range(10):
            size = size/10
            cca_val = int(float(cca1[d['w_i']][k])/size)
            phrase = 'cca:current,pos={0},size={1},bucket={2},t={3}'.format(k,size,cca_val,d['t'])
            index = phi.get(phrase, -1)
            if index == -1:
                continue
            positions.append((index,1))
#    for i in range(cca_length):
#        if cca1.get(d['w_m1'],-1) == -1:
#            break
#        phrase = 'cca:previous,pos={0},t={1}'.format(i, d['t'])
#        index = phi.get(phrase, -1)
#        if not index == -1:
#            temp = float(cca1[d['w_m1']][i])
#            factor = temp
#            if temp > 0:
#                factor = math.log(1+temp)
#            elif temp < 0:
#                factor = -1*math.log(1+(-1*temp))
#            positions.append((index,beta*factor))
#    for i in range(cca_length):
#        if cca1.get(d['w_i'],-1) == -1:
#            break
#        phrase = 'cca:current,pos={0},t={1}'.format(i, d['t'])
#        index = phi.get(phrase, -1)
#        if not index == -1:
#            temp = float(cca1[d['w_i']][i])
#            factor = temp
#            if temp > 0:
#                factor = math.log(1+temp)
#            elif temp < 0:
#                factor = -1*math.log(1+(-1*temp))
#            positions.append((index,beta*factor))
    for i in regExp:
        if i.match(d['w_i']):
            phrase = 're={0},t={1}'.format(regExp[i],d['t'])
            index = phi.get(phrase, -1)
            if index == -1:
                continue
            positions.append((index,1))
    if Words.get(d['w_i'], False) and Words[d['w_i']][0] < 5:
        phrase = 'rare=_RARE_,t={0}'.format(d['t'])
        index = phi.get(phrase, -1)
        if not index == -1:
            positions.append((index,1))        
    if len(d['w_i']) > 4 and not d['w_i'] == '_STOP_':
        phrase = 'last_four={0},t={1}'.format(d['w_i'][len(d['w_i'])-4:len(d['w_i'])],d['t'])
        index = phi.get(phrase, -1)
        if not index == -1:
            positions.append((index,1))
    if codes.get(d['w_i'], False) and not d['w_i'] == '_STOP_':
        code = codes[d['w_i']]
        for c_len in range(2, len(code), 2):
            st = code[:c_len]
            phrase = 'code={0},t={1}'.format(code[:c_len],d['t'])
            index = phi.get(phrase, -1)
            if not index == -1:
                positions.append((index,1))
        st = code
        phrase = 'code={0},t={1}'.format(code,d['t'])
        index = phi.get(phrase, -1)
        if not index == -1:
            positions.append((index,1))        
    return copy.deepcopy(positions)

def viterbi(sent, part_speach, phi, tags, alpha, strings, Words, regExp, codes, cca1, cca_length):
    pi.clear()
    bp.clear()
    r = len(sent)
    sentence = ['_*_', '_*_'] + sent + ['_STOP_', '_STOP_', '_STOP_']
    POS = [('*','*'),('*','*')] + part_speach + [('_STOP','_STOP_'),('_STOP','_STOP_'),('_STOP','_STOP_')]
    pi[(0, '*', '*')] = 1.0
    for k in range(1,r+1):
        t1 = tags
        t2 = tags
        t3 = tags
        if k == 1:
            t1 = ['*']
            t3 = ['*']
        if k == 2:
            t3 = ['*']
        if Words.get(sentence[k+1], False) and Words[sentence[k+1]][0] > 5:
            t2 = list(Words[sentence[k+1]][1])
        for u in t1:
            for v in t2:
                for w in t3:
                    pi_val = pi.get((k-1,w,u), 0.5)
                    if pi_val == 0.5:
                        continue
                    d = dict(w_m2 = sentence[k-1], w_m1 = sentence[k], w_i = sentence[k+1], w_1 = sentence[k+2], w_2 = sentence[k+3], t_2 = w, t_1 = u, t = v, pos = POS[k+1][0], chnk = POS[k+1][1])
                    indices = get_alpha_indices(strings, phi, d, Words, regExp, codes, cca1, cca_length)
                    val = pi_val
                    for i in indices:
                        val += alpha[i[0]]*i[1]
                    test = pi.get((k,u,v), 0.5)
                    if test == 0.5 or val > test:
                        pi[(k,u,v)] = val
                        bp[(k,u,v)] = w
    result_tags = []
    result_val = 0
    got_first = False
    tags2 = tags
    if r == 1:
        tags2 = ['*']
    for u in tags2:
        for v in tags:
            pi_val = pi.get((r,u,v), 0.5)
            if pi_val == 0.5:
                continue
            d = dict(w_m2 = sentence[r], w_m1 = sentence[r+1], w_i = sentence[r+2], w_1 = sentence[r+3], w_2 = sentence[r+4], t_2 = u, t_1 = v, t = 'STOP', pos = POS[r+2][0], chnk = POS[r+2][1])
            indices = get_alpha_indices(strings, phi, d, Words, regExp, codes, cca1, cca_length)
            val = pi_val
            for i in indices:
                val += alpha[i[0]]*i[1]
            if val > result_val or not got_first:
                got_first = True
                result_tags = [v,u]
                result_val = val
    for k in range(r-2, 0, -1):
        t = bp[(k+2, result_tags[len(result_tags)-1], result_tags[len(result_tags)-2])]
        result_tags.append(t)
    result_tags.reverse()
    while result_tags[0] == '*':
        result_tags.reverse()
        result_tags.pop()
        result_tags.reverse()
    return copy.deepcopy(result_tags)
