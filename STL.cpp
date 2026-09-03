#include <iostream>
#include <vector>
using namespace std;



int main(int argc, char *argv[]) {
	vector<int> v = {10, 20, 30, 40, 50};
	
	vector<int> iterator it = v.begin();
	
	cout<<"Elemento inicial"<<*it<<endl;
	it+=3;
	cout<<"cuarto elemento"<<*it<<endl;
	
	return 0;
}

