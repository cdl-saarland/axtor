#!/usr/bin/python

from os import walk
import shlex, subprocess  

testfiles = [filenames for (dirpath, dirnames, filenames) in walk("src/")][0]

testTimeout = 1000

def shellCmd(cmdText, logPrefix):
    cmd = shlex.split(cmdText)
    with open(logPrefix + ".out", "w") as fOut:
        with open(logPrefix + ".err", "w") as fErr:   
            proc = subprocess.Popen(cmd, stdout=fOut, stderr=fErr)
            retCode=proc.wait(timeout = testTimeout)
            if retCode != 0:
                print(cmdText)
            return retCode

def compile_to_LLVM(srcFile, destFile, suffix):
    return shellCmd("clang -S -emit-llvm " + srcFile + " -o " + destFile, "logs/gcc_" + suffix) == 0
    
def compile_to_obj(srcFile, destFile, suffix):
    return shellCmd("icc -c " + srcFile + " -o " + destFile, "logs/gcc_" + suffix) == 0

def axtor_run(srcFile, destFile, backend, suffix):
    return shellCmd("axtor -i " + srcFile + " -o " + destFile + " -m " + backend, "logs/axtor_" + suffix) == 0

def runTest(file):
    # TODO
    baseName = file
    suffix = baseName

    generatedFilePath = "build/" + baseName + "_axtor.cpp"
    compiledFilePath = "build/" + baseName + "_axtor.o"

    print (file[:-2])

    if file[-2:]==".c":
        testFilePath = "src/" + file
        llvmFilePath = "build/" + baseName + ".ll"            
    
        print("..clang {0}".format(file))
        if not compile_to_LLVM(testFilePath, llvmFilePath, suffix):
            return False

    elif file [-3:]==".ll" or file[-3:] == ".bc":
        llvmFilePath = "src/" + file
        testFilePath = llvmFilePath
    
    print("..axtor {0}".format(file))
    if not axtor_run(llvmFilePath, generatedFilePath, "C", suffix):
        return False
    
    print("..gcc {0}".format(file))
    if not compile_to_obj(generatedFilePath, compiledFilePath, suffix):
        return False
    
    
    # TODO compare against directly compiled version!
    return True

for file in testfiles:
    print("Test {0}".format(file))
    if runTest(file):
        print("passed!")
    else:
        print("failed!")
    
    

