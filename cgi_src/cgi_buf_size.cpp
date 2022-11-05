#include <iostream>

using namespace std;
int main(int argc, char **argv, char **envp){
	cout << "content-type:text/plain\r\n\r\n";
	for (int i = 0; i < 4096 - 28; ++i){
		cout << "a";
	}
	cout << endl;
}
