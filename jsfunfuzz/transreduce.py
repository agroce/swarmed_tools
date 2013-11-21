import subprocess
import sys
import os
import operator
import runJS as R

js = "/scratch/js1.6/src/Linux_All_DBG.OBJ/js"
timeout = 5
outfile = None

test = sys.argv[1]
outfn = sys.argv[2]
for o in sys.argv[1:]:
    if o.find("--js=") == 0:
        js = o.split("--js=")[1]
    elif o.find("--out=") == 0:
        outfile = o.split("--out=")[1]
    elif o.find("--timeout=") == 0:
        timeout = int(o.split("--timeout=")[1])

anyRemoved = True

if outfile == None:
    (failure, rcode) = R.getOutputReturn(test, js, timeout)
else:
    failure = []
    for l in open(outfile):
        failure.append(l)

def checkFail(text):
    tname = str(os.getpid()) + ".test"
    tfile = open(tname,'w')
    tfile.write(text)
    tfile.close()
    success = R.failSame(tname, failure, operator.eq, js, timeout)
    os.remove(tname)
    return success

text = ""
for l in open(test):
    text = text + l

equalitySet = []
pos = -1

breakers = ' ()[]{};"'
changed = True

while changed:
    changed = False

    tpos = 0

    for line in text.split("\n"):
        pos = 0
        for eq in xrange(0,line.count("=")):
            pos = line.find("=",pos+1)
            lhsPos = reduce(max,filter(lambda x: x >= 0, map(lambda x: line.rfind(x,0,pos), breakers)))
            rhsPos = reduce(min,filter(lambda x: x >= 0, map(lambda x: line.find(x,pos), breakers)))
            lhs = line[lhsPos+1:pos]
            rhs = line[pos+1:rhsPos]
            place = text.find(lhs + "=" + rhs, tpos)
            equalitySet.append((lhs, rhs, place))
        tpos = tpos + len(line) + 1

    stillFail = []
    for (lhs, rhs, lp) in equalitySet:
        firstText = text[0:lp]
        secondText = text[lp:]
        secondText = secondText.replace(lhs+"="+rhs,"")
        secondText = secondText.replace(lhs,rhs)
        newText = firstText + secondText
        if checkFail(newText):
            stillFail.append(newText)

    allFails = []
    for t in stillFail:
        lines = t.split("\n")
        allFails.append(t)
        for p in xrange(0,len(lines)):
            tryL = lines[0:p]
            tryL.extend(lines[p+1:])
            if len(tryL) > 1:
                tryText = reduce(lambda x,y: x+"\n"+y, tryL)
            elif len(tryL) == 1:
                tryText = tryL[0]
            else:
                continue
            if checkFail(tryText):
                allFails.append(tryText)

    if len(allFails) > 0:
        allFails = sorted(allFails, key=lambda x:len(x))

        shortest = allFails[0]
        if shortest != text:
            text = shortest
            changed = True

outf = open(outfn,'w')
outf.write(text)
if text[-1] != "\n":
    outf.write("\n")
outf.close()
    
                
                        
