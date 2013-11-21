# $Id: GCCDD.py,v 1.1 2001/11/05 19:53:33 zeller Exp $
# Using delta debugging on GCC input
# Modified to perform minimization on jsfunfuzz tests by Alex Groce

import DD
import runJS as R
import string
import os
import operator
import sys
import time

def parenSplitInner(string):
    tryBegin = string[0:10]
    tryEnd = string[-3:]
    string = string[10:-3]
    splits = [tryBegin]
    leftParens = "([{"
    rightParens = ")]}"
    if string == "":
        return splits
    pos = 0
    lastLeft = 0
    leftMark = ""
    rightMark = ""
    while pos < len(string):
        if (string[pos] in leftParens):
            leftMark = string[pos]
            rightMark = rightParens[leftParens.find(leftMark)]
            lastLeft = pos
            pos = pos + 1
        elif (string[pos] == rightMark):
            splits.append(string[:lastLeft])
            splits.append(string[lastLeft:pos+1])
            string = string[pos+1:]
            rightMark = ""
            pos = 0
        else:
            pos = pos + 1
    splits.append(string)
    splits.append(tryEnd)
    return splits                

def tokenSplit(string,direction):
    if direction == "parens":
        return parenSplitInner(string)
    tryBegin = string[0:10]
    tryEnd = string[-3:]
    string = string[10:-3]
    splits = [tryBegin]
    if string == "":
        return splits
    standAlone = " ()[]{};"
    if direction == "left":
        leftBinders = ":.,=-+*/&|<>"
        rightBinders = ""
    elif direction == "right":
        rightBinders = ":.,=-+*/&|<>"
        leftBinders = ""
    elif direction == "alone":
        rightBinders = ""
        leftBinders = ""
        standAlone = standAlone + ":.,=-+*/&|<>"
    elif direction == "camel":
        standAlone = standAlone + ":.,=-+*/&|<>"
        rightBinders = "ABCDEFGHIJKLMNOPQRSTUVWXYZ"
        leftBinders = ""
    pos = 0
    while pos < len(string):
        if string[pos] in standAlone:
            splits.append(string[:pos])
            splits.append(string[pos])
            string = string[pos+1:]
            pos = 0
        elif string[pos] in leftBinders:
            splits.append(string[:pos+1])
            string = string[pos+1:]
            pos = 0
        elif string[pos] in rightBinders:
            if(pos > 0):
                splits.append(string[:pos])
                string = string[pos:]
                pos = 0
            else:
                pos = pos + 1
        else:
            pos = pos + 1
    splits.append(string)
    splits.append(tryEnd)
    return splits


class MyDD(DD.DD):
    originalOutput = []
    js = "/scratch/js1.6/src/Linux_All_DBG.OBJ/js"
    outfile = None
    timeout = 5
    verbose = False
    
    def __init__(self):
        DD.DD.__init__(self)
        
    def _test(self, deltas):

        elapsed = time.time() - mydd.startTime
        if (elapsed > mydd.timeleft):
            os.exit(1)
            return self.PASS
        
        # Build input
        input = ""
        for (index, delta) in deltas:
            input = input + delta

        tname = str(os.getpid()) + ".test"
        # Write input to `input.c'
        out = open(tname, 'w')
        out.write(input)
        out.close()

        #print self.coerce(deltas)

        result = R.failSame(tname, self.originalOutput, operator.eq, self.js, self.timeout, self.verbose)
        os.remove(tname)

        if result:
            return self.FAIL
        else:
            return self.PASS

    def coerce(self, deltas):
        # Pretty-print the configuration
        input = ""
        for (index, character) in deltas:
            input = input + character
        return input


if __name__ == '__main__':
    # Usage:  <file to minimize> <output file> --js=location --out=outputFile --timeout=timeout --verbose

    mydd = MyDD()
    
    toMin = sys.argv[1]
    minOut = sys.argv[2]

    mode = "lines"
    tokens = False
    
    for o in sys.argv[1:]:
        if o.find("--js=") == 0:
            mydd.js = o.split("--js=")[1]
        elif o.find("--out=") == 0:
            mydd.outfile = o.split("--out=")[1]
        elif o.find("--timeout=") == 0:
            mydd.timeout = int(o.split("--timeout=")[1])
        elif o.find("--timeleft=") == 0:
            mydd.timeleft = float(o.split("--timeleft=")[1]) 
        elif o.find("--mode") == 0:
            mode = o.split("--mode=")[1]
        elif o.find("--verbose") == 0:
            mydd.verbose = True

    mydd.startTime = time.time()
    
    deltas = []
    index = 1

    if mode == "lines":
        fileContents = open(toMin).readlines()
    elif mode == "chars":
        fileContents = open(toMin).read()
    elif mode == "parens":
        tokens = True
        tokenMode = "parens"
    elif mode == "camel":
        tokens = True
        tokenMode = "camel"
    elif mode == "left":
        tokens = True
        tokenMode = "left"
    elif mode == "right":
        tokens = True
        tokenMode = "right"

    if tokens:
        lines = open(toMin).readlines()
        fileContents = []
        for l in lines:
            fileContents.extend(tokenSplit(l, tokenMode))
    
    for delta in fileContents:
        deltas.append((index, delta))
        index = index + 1

    if mydd.outfile == None:
        (mydd.originalOutput, rcode) = R.getOutputReturn(toMin, mydd.js, mydd.timeout)
    else:
        for l in open(mydd.outfile):
            mydd.originalOutput.append(l)

    c = mydd.ddmin(deltas)              # Invoke DDMIN

    text = mydd.coerce(c)

    outf = open(minOut,'w')
    outf.write(text)
    if (mode == "chars") and (text[-1] != "\n"):
        outf.write("\n")
    outf.close()


