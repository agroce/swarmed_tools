import runJS as R
import sys
import os
import subprocess

def dumpTest():
    for l in open(testOut):
            print "  " + l,

js = "/scratch/js1.6/src/Linux_All_DBG.OBJ/js"
timeout = 5

testIn = sys.argv[1]
testOut = sys.argv[2]

os.environ["COLLECT"] = "False"
collect = False

dnull = open(os.devnull,'w')

for o in sys.argv[2:]:
    if o.find("--js=") == 0:
        js = o.split("--js=")[1]
    if o.find("--timeout=") == 0:
        timeout = int(o.split("--timeout=")[1])
    if o.find("--collect=") == 0:
        collect = True
        clen = o.split("--collect=")[1]

# Create an output file for all minimizing

(outlines, rcode) = R.getOutputReturn(testIn, js, timeout)

tr = str(os.getpid())
tmpOut = tr + ".out"
tmpOutF = open(tmpOut, 'w')
for l in outlines:
    tmpOutF.write(l)
tmpOutF.close()

tmpTest = tr + ".test"

if collect:
    os.environ["COLLECT"] = "True"
    os.environ["SUCCFILE"] = testOut + ".succ"
    os.environ["DIFFFILE"] = testOut + ".diff"
    os.environ["FAILFILE"] = testOut + ".fail"
    os.environ["MAXCLEN"] = clen
    subprocess.call(['echo "++" > ' + os.environ["SUCCFILE"]], shell=True)
    subprocess.call(['echo "++" > ' + os.environ["DIFFFILE"]], shell=True)
    subprocess.call(['echo "++" > ' + os.environ["FAILFILE"]], shell=True)
    
print "Initial line minimization..."
# First do line minimization, outside the main loop
subprocess.call(["python","jsDD.py", testIn, testOut, "--js="+js, "--timeout="+str(timeout), "--out="+tmpOut])

print "Trivial reductions (strings, comments, semicolons)..."
subprocess.call(["python","trivialreduce.py", testOut, testOut, "--js="+js, "--timeout="+str(timeout), "--out="+tmpOut])

print "INITIAL MINIMIZATION:"
dumpTest()

allPasses = [("transitive", "transreduce.py", []),
          ("parentheses", "parenreduce.py", []),
          ("DD-parentheses", "jsDD.py", ["--mode=parens"]),
          ("DD-left", "jsDD.py", ["--mode=left"]),
          ("DD-right", "jsDD.py", ["--mode=right"]),
          ("DD-camel","jsDD.py", ["--mode=camel"]),
          ("DD-chars","jsDD.py", ["--mode=chars"])]

remaining = allPasses

changed = True

while changed:
    subprocess.call(["cp",testOut,tmpTest])
    alreadyRun = []
    changed = False
    for (name, file, args) in remaining:
        print "RUNNING PASS:", name
        toCall = ["python", file, testOut, testOut, "--js="+js, "--timeout="+str(timeout), "--out="+tmpOut]
        toCall.extend(args)
        subprocess.call(toCall)
        if subprocess.call(["diff",testOut,tmpTest],stdout=dnull,stderr=dnull):
            print "CHANGED TO:"
            dumpTest()
            alreadyRun = (name, file, args)
            changed = True
            break
    if changed:
        remaining = list(allPasses)
        remaining.remove(alreadyRun)

os.remove(tmpTest)
os.remove(tmpOut)

print "DONE"
