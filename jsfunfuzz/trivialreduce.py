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

pos = 0

# First try to remove all comments

startComment = text.find("/*")
commentPos = 0
while startComment != -1:
    endComment = text.find("*/")
    newText = text[:startComment] + " " + text[endComment+2:]
    if checkFail(newText):
        text = newText
        commentPos = 0
    else:
        commentPos = endComment
    startComment = text.find("/*",commentPos)

# Next, strings

startComment = text.find('\"')
commentPos = 0
while startComment != -1:
    endComment = text.find('\"', startComment+1)
    newText = text[:startComment+2] + text[endComment+2:]
    if checkFail(newText):
        text = newText
        commentPos = 0
    else:
        commentPos = endComment
    startComment = text.find("\"",commentPos)    

text = text.replace(");\n",")\n")

outf = open(outfn,'w')
outf.write(text)
if text[-1] != "\n":
    outf.write("\n")
outf.close()
    
                
                        
