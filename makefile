ThreadPool:main.cc ThreadPool.hpp
	g++ -o $@ $^ -g -lpthread -fpermissive 
.PHONY:clean
clean:
	rm -f ThreadPool 
