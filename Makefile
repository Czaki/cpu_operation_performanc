fastestrng:  benchmark.cpp
	c++ --version
	c++ -std=c++17 -O2 -fno-tree-vectorize -o benchmark.out benchmark.cpp  -I include -Wall -Wextra
	echo "run sudo ./benchmark.out"
clean:
	rm -r -f benchmark
