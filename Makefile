serverM: serverM.cpp
	clang++ serverM.cpp -o serverM

serverEE: serverEE.cpp
	clang++ serverEE.cpp -o serverEE

serverCS: serverCS.cpp
	clang++ serverCS.cpp -o serverCS

client: client.cpp
	clang++ client.cpp -o client