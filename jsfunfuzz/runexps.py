import subprocess

rtime = '21600'


dd = 'mkdir withdd/30m.X ; python runtests.py "withdd/30m.X/t" 100000000 ' + rtime + ' "/scratch/js1.6/src/Linux_All_DBG.OBJ/js" "/scratch/js1.6/src/Linux_All_DBG.OBJ/" "/scratch/js1.6/src/" "Linux_All_DBG.OBJ" --reduce >& withdd/30m.X/run'

throttledd = 'mkdir throttle6hour/30m.X ; python runtests.py "throttle6hour/30m.X/t" 100000000 ' + rtime + ' "/scratch/js1.6/src/Linux_All_DBG.OBJ/js" "/scratch/js1.6/src/Linux_All_DBG.OBJ/" "/scratch/js1.6/src/" "Linux_All_DBG.OBJ" --reduce --throttle >& throttle6hour/30m.X/run'

nodd = 'mkdir nodd6hour/30m.X ; python runtests.py "nodd6hour/30m.X/t" 100000000 ' + rtime + ' "/scratch/js1.6/src/Linux_All_DBG.OBJ/js" "/scratch/js1.6/src/Linux_All_DBG.OBJ/" "/scratch/js1.6/src/" "Linux_All_DBG.OBJ" >& nodd6hour/30m.X/run'


print dd
print nodd

for n in xrange(7,12):
    print "ITERATION ",n
    rundd = dd.replace("X", str(n))
    runthrottledd = throttledd.replace("X", str(n))
    runnodd = nodd.replace("X", str(n))
    
    #subprocess.call([rundd], shell=True)
    #subprocess.call(["rm -rf *.out"], shell=True)
    subprocess.call([runthrottledd], shell=True)
    subprocess.call(["rm -rf *.out"], shell=True)
    subprocess.call([runnodd], shell=True)
    subprocess.call(["rm -rf *.out"], shell=True)
