import sys
import random

infn = sys.argv[1]
outfn = sys.argv[2]
confn = sys.argv[3]

configured = False
biased = False

for opt in sys.argv:
    if "--seed" in opt:
        random.seed(int(opt.split("=")[1]))
    if "--config" in opt:
        config = (opt.split("=")[1]).split()
        configured = True
    if "--bias" in opt:
        biasf = (opt.split("=")[1])
        biased = True

if biased:
    bias = {}
    for l in open(biasf):
        ls = l.split()
        f = int(ls[0])
        num = int(ls[1])
        den = int(ls[2])
        bias[f] = (num,den) 

outf = open(outfn,'w')
outcf = open(confn, 'w')

n = 0

for line in open(infn):
    l = line.split()
    if ((l != []) and ((l[0] == "function(dr)") or
                       (l[0] == "function(depth)")) and("}," in line)):
        n = n + 1
        if not configured:
            (p1,p2) = (1,2)
            if biased:
                (p1,p2) = bias[n]
            if (random.randint(0,p2-1) < p1):
                outf.write(line)
                outcf.write("--" + str(n) + " ")
        else:
            if str(n) in config:
                outf.write(line)
                outcf.write("--" + str(n) + " ")                
    elif ((l != []) and (l[0] == "case")):
        n = n + 1
        if not configured:
            (p1,p2) = (1,2)
            if biased:
                (p1,p2) = bias[n]
            if (random.randint(0,p2-1) < p1):
                outf.write(line)
                outcf.write("--" + str(n) + " ")
        else:
            if str(n) in config:
                outf.write(line)
                outcf.write("--" + str(n) + " ") 
    else:
        outf.write(line)
