import subprocess
import time
import os
import signal

jsDefault = "/scratch/js1.6/src/Linux_All_DBG.OBJ/js"
timeoutDefault = 5

def genTest(test, tname):
    subprocess.call(["cp", "jsfunrun.js", tname]) # Need jsfunfuzz definitions to run test
    subprocess.call(["cat " + test + ">>" + tname], shell=True)
    subprocess.call(["echo \"\ndumpln(\\\"TEST COMPLETED SUCCESSFULLY\\\");\" >> " + tname], shell=True)

def runTest(js, test="run.js", out="run.out", timeout=7):
    start = time.time()
    elapsed = time.time() - start
    p = subprocess.Popen(["script -c \"" + js + " " + test + "\" " + out + " >& /dev/null"], shell=True)
    while (p.poll() is None) and (elapsed < timeout):
        elapsed = time.time() - start
    if (p.poll() is None):
        print "TERMINATING"
        os.kill(p.pid, signal.SIGKILL)
    return p.returncode

def genAndRun(test, js = jsDefault, timeout = timeoutDefault):
    root = "test." + str(os.getpid())
    tname = root + ".js"
    oname = root + ".out"
    genTest(test, tname)
    rcode = runTest(js, tname, oname, timeout)
    return (rcode, tname, oname)

def relevant(line):
    if "before" in line and "after" in line and "break" in line:
        return False
    if line.find("Targeting") == 0:
        return False
    if line.find("Compiling threw") == 0:
        return False
    if line.find("Running threw") == 0:
        return False
    if line.find("Script") == 0:
        return False
    if line == "\n":
        return False
    if line == "\r\n":
        return False
    return True

def cleanup(output):
    newOutput = []
    for l in output:
        if l == "Finished this step.\r\n":
            newOutput = []
        elif l.find("Script") != 0:
            newOutput.append(l)
    return newOutput

def getOutputReturn(test, js = jsDefault, timeout = timeoutDefault):
    (rcode, tname, oname) = genAndRun(test, js, timeout)
    ofile = open(oname, 'r')
    output = cleanup(ofile.readlines())
    ofile.close()
    if (("TEST COMPLETED SUCCESSFULLY\r\n") not in output):
        badTest = False
        for l in output:
            if "SyntaxError" in l:
                if "throw" not in l:
                    badTest = True
        if not badTest:
            subprocess.call(['grep  "^tryItOut" ' + tname + " > " + os.environ["COLLECTOUT"] + "." + os.environ["COLLECTIND"] + ".test"], shell=True)
            subprocess.call(["cp " + oname + " " + os.environ["COLLECTOUT"] + "." + os.environ["COLLECTIND"] + ".out"], shell=True)
            os.environ["COLLECTIND"] = str(1 + int(os.environ["COLLECTIND"]))

    os.remove(tname)
    os.remove(oname)
    return (output, rcode)

def failSame(test, ocheck, compareFn, js = jsDefault, timeout = timeoutDefault, verbose=False):
    tlen = 0
    for l in open(test):
        tlen += 1
    (output, rcode) = getOutputReturn(test, js, timeout)
    if "TEST COMPLETED SUCCESSFULLY\r\n" in output:
        if verbose:
            print "SUCCESSFUL EXECUTION"
        return False
    for n in xrange(0,min(len(output), len(ocheck))):
        if verbose:
            print "COMPARING (ORIGINAL): ",ocheck[n][:-1]
            print "       WITH (OUTPUT): ",output[n][:-1]
        if not compareFn(output[n], ocheck[n]):
            return False

    return True
    
