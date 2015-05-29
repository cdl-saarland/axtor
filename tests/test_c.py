#!/usr/bin/python

from os import walk
import shlex, subprocess  

testfiles = [filenames for (dirpath, dirnames, filenames) in walk("src/")][0]

testTimeout = 1000

def shellCmd(cmdText, logPrefix):
    # print(cmdText)
    cmd = shlex.split(cmdText)
    with open(logPrefix + ".out", "w") as fOut:
        with open(logPrefix + ".err", "w") as fErr:   
            proc = subprocess.Popen(cmd, stdout=fOut, stderr=fErr)
            return proc.wait(timeout = testTimeout)
                            
    return proc.returncode

def compile_to_LLVM(srcFile, destFile, suffix):
    return shellCmd("clang -S -emit-llvm " + srcFile + " -o " + destFile, "logs/gcc_" + suffix) == 0
    
def compile_to_obj(srcFile, destFile, suffix):
    return shellCmd("g++ -c " + srcFile + " -o " + destFile, "logs/gcc_" + suffix) == 0

def axtor_run(srcFile, destFile, backend, suffix):
    return shellCmd("axtor -i " + srcFile + " -o " + destFile + " -m " + backend, "logs/axtor_" + suffix) == 0
    
    
    

for file in testfiles:
    print("Test {0}".format(file))
    
    # TODO
    baseName = file
    suffix = "test"
    
    testFilePath = "src/" + file
    llvmFilePath = "build/" + baseName + ".ll"            
    generatedFilePath = "build/" + baseName + "_axtor.c"
    compiledFilePath = "build/" + file + "_axtor.o"
    
    print("..clang {0}".format(file))
    if not compile_to_LLVM(testFilePath, llvmFilePath, suffix):
        print("failed!")
        continue
    
    print("..axtor {0}".format(file))
    if not axtor_run(llvmFilePath, generatedFilePath, "C", suffix):
        print("failed!")
        continue
    
    print("..gcc {0}".format(file))
    if not compile_to_obj(generatedFilePath, compiledFilePath, suffix):
        print("failed!")
        continue
    
    
    # TODO compare against directly compiled version!
    
    print("passed!")
    

