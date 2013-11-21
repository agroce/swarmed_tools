import subprocess
import os
import math

def valueFor(v):
    return math.log(1/v)

def getCoverage(gcovDir, objDir, save=False, prefix="", prune=False, bprune={}, lprune={}, fprune={}, value=False):
    subprocess.call(["cd " + gcovDir + "; gcov *.c *.cpp -b -o " + objDir + ">& /dev/null"], shell=True)

    if value:
        vSum = 0.0

    fcov = []
    lcov = []
    bcov = []
    gcovFiles = os.listdir(gcovDir)
    gcovFiles = sorted(gcovFiles)
    for f in gcovFiles:
        lastline = "BADLINE"
        if ".gcov" in f:
            for l in open(gcovDir + f):
                fs = l.split()
                if fs[0] == "function":
                    if int(fs[3]) > 0:
                        if (not prune) or (fs[1] in fprune):
                            fcov.append(fs[1])
                            if value:
                                vSum += valueFor(fprune[fs[1]])
                elif fs[0] == "branch":
                    if "never" not in fs and "0%" not in fs:
                        perc = ""
                        for ps in fs:
                            if "%" in ps:
                                perc = ps[:-1]
                                bcentry = f + ":" + lastline + ":b" + fs[1]
                                if (not prune) or (bcentry in bprune):
                                    bcov.append(bcentry)
                                    if value:
                                        vSum += valueFor(bprune[bcentry])
                elif fs[0] != "call":
                    ls = l.split(":")
                    ls0 = ls[0].split()[0]
                    ls1 = (ls[1].split()[0]).split(".gcov")[0]
                    lastline = ls1
                    if (ls0 != "-") and (ls0 != "#####"):
                        lcentry = f.split(".gcov")[0] + ":" + ls1
                        if (not prune) or (lcentry in lprune):
                            lcov.append(lcentry)
                            if value:
                                vSum += valueFor(lprune[lcentry])

    if save:
        fcovFile = open(prefix + ".fcov",'w')
        bcovFile = open(prefix + ".bcov",'w')
        lcovFile = open(prefix + ".lcov",'w')

        for f in fcov:
            fcovFile.write(f + "\n")
        fcovFile.close()

        for b in bcov:
            bcovFile.write(b + "\n")
        bcovFile.close()

        for l in lcov:
            lcovFile.write(l + "\n")
        lcovFile.close()

    if value:
        print "VALUE = " + str(vSum)
        valf = open(prefix + ".value", 'w')
        valf.write(str(vSum) + "\n")

    return (lcov, bcov, fcov)
