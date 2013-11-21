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

while anyRemoved:
    anyRemoved = False
    pos = 0
    target = 0
    leftParens = "([{<"
    rightParens = ")]}>"
    binops = ":.,=-+*/&|<>"
    nth = 0
    nesting = 0
    leftMark = ""
    rightMark = ""
    targetN = text.count("(") + text.count("[") + text.count("{") + text.count("<")
    while (target < targetN) and (not anyRemoved):
        while pos < len(text):
            if (leftMark == "") and (text[pos] in leftParens):
                if (nth == target):
                    leftpos = pos
                    leftMark = text[pos]
                    rightMark = rightParens[leftParens.find(leftMark)]
                    nesting = 0
                else:
                    nth = nth + 1
            elif (leftMark != "") and (text[pos] == rightMark):
                if nesting == 0:
                    if (leftpos -1 >= 0) and (text[leftpos-1] in binops):
                        newText = text[0:leftpos-1] + text[pos+1:]
                        if checkFail(newText):
                            text = newText
                            anyRemoved = True
                            break
                    if (pos+1 < len(text)) and (text[pos+1] in binops):
                        newText = text[0:leftpos-1] + text[pos+2:]
                        if checkFail(newText):
                            text = newText
                            anyRemoved = True
                            break                        
                    newText = text[0:leftpos] + text[pos+1:]
                    if checkFail(newText):
                        text = newText
                        anyRemoved = True
                        break
                    newText = text[0:leftpos] + text[leftpos+1:pos] + text[pos+1:]
                    if checkFail(newText):
                        text = newText
                        anyRemoved = True
                        break
                else:
                    nesting = nesting - 1
            elif (text[pos] == leftMark):
                nesting = nesting + 1
            pos = pos + 1
        if not anyRemoved:
            target = target + 1
            pos = 0
            nth = 0
            nesting = 0
            leftMark = ""
            rightMark = ""

outf = open(outfn,'w')
outf.write(text)
if text[-1] != "\n":
    outf.write("\n")
outf.close()
    
                
                        
            
