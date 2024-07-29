all:
	g++ -std=c++11 -o serverM serverM.cpp
	g++ -std=c++11 -o serverS serverS.cpp
	g++ -std=c++11 -o serverL serverL.cpp
	g++ -std=c++11 -o serverH serverH.cpp
	g++ -std=c++11 -o client client.cpp

clean:
	$(RM) serverM serverS serverL serverH client