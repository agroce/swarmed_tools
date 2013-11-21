import subprocess
import sys
import time
import os
import runJS as R
import coverage
import random

prefix = sys.argv[1]
maxRuns = int(sys.argv[2])
timeout = int(sys.argv[3])
js = sys.argv[4]
gcdaDir = sys.argv[5]
gcovDir = sys.argv[6]
objDir = sys.argv[7]

printInterval = 10
failures = 0

swarm = "--noSwarm" not in sys.argv
runReduce = "--reduce" in sys.argv
throttle = "--throttle" in sys.argv

subprocess.call(["rm -rf " + gcdaDir + "/*.gcda"], shell=True)

if throttle:
    sigcores = ["Assertion failure", "received signal", "Mismatch with uneval/eval",
                "The error was: ReferenceError: Iterator is not defined"]
    signatures = {}

n = 1

startTime = time.time()
print "STARTING AT", startTime

elapsed = 0.0

while (n != (maxRuns+1)) and (elapsed < timeout):
    elapsed = time.time() - startTime
    if (n % printInterval) == 0: 
        print "TEST " + str(n), "; ELAPSED:", elapsed, "FAILURES:", failures
    
    testCase = prefix + ".tc" + str(n)

    # First, create the swarmed generator
    cName = testCase + "gen.js"
    cfName = testCase + ".conf"

    if swarm:
        subprocess.call(["python swarmup.py jsfunswarm.js " + cName + " " + cfName], shell=True)
    else:
        subprocess.call(["cp jsfunswarm.js " + cName], shell=True)

    # Generate test case
    subprocess.call([js + " " + cName + " >& " + testCase + ".orig"], shell=True)
    subprocess.call(["grep tryItOut " + testCase + ".orig >& " + testCase],
                    shell=True)
    os.remove(testCase + ".orig")

    (rcode, tname, oname) = R.genAndRun(testCase, js)

    status = subprocess.call(["grep \"TEST COMPLETED SUCCESSFULLY\" " + oname + " >& /dev/null"],
                             shell=True)

    doReduce = True
    if throttle:
        for l in open(oname):
            foundSig = False
            for v in sigcores:
                if v in l:
                    sig = l[:-1]
                    foundSig = True
                    break
            if foundSig:
                if sig in signatures:
                    signatures[sig] += 1
                else:
                    print "New failure signature: " + sig
                    signatures[sig] = 1
                doReduce = random.randint(1,signatures[sig]) == 1
                break

    if (status != 0):
        print "TEST",n,"FAILED"
        failures += 1
        subprocess.call(['grep "^tryItOut" ' + tname + " >  " + testCase+".test"], shell=True)
        subprocess.call(["cp " + oname + " " + testCase + ".out"],shell=True)
        if runReduce and doReduce:
            timeleft = timeout - (time.time() - startTime)
            subprocess.call(["python jsreduce.py " + testCase+".test" + " " + testCase+".test.min --js=" + js + " --timeleft=" + str(timeleft)], shell=True)

    os.remove(tname)
    os.remove(oname)
    n = n + 1

(bcov, lcov, fcov) = coverage.getCoverage(gcovDir, objDir, True, prefix)

print "TOTAL TESTS: ", n-1
print "TOTAL FAILURES: ", failures
print "FUNCTION COVERAGE: ", len(fcov)
print "LINE COVERAGE: ", len(lcov)
print "BRANCH COVERAGE: ", len(bcov)

