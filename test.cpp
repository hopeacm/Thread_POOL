#include"ThreadPool.h"
#include <iostream>
#include <vector>
#include <chrono>
using namespace std;

int main()
{
	ThreadPool pool(4);
	vector<future<int> > results;
	for (int i = 0; i < 8; ++i) {
		results.emplace_back(  //emplace_back与push_back相似，只是性能较好
			pool.enqueue([i] {
				cout << "hello " <<" "<< i << endl;
				this_thread::sleep_for(chrono::seconds(1));
				cout << "world " << " " << i << endl;
				return i * i;
			}));
	}
	for (auto && result : results)
		cout << result.get() << ' ';
	cout << endl;
	return 0;
}
