all:
	clang -ccc-host-triple ptx32 -fno-builtin -emit-llvm -c OCL_FrontendWrapper.cpp -o OCL_FrontendWrapper.bc
	clang -ccc-host-triple ptx32 -fno-builtin -O3 -emit-llvm -c builtins.cpp -o OCL_builtins.bc

.PHONY: clean

clean:
	rm -f OCL_FrontendWrapper.bc
	
