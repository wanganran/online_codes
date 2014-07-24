#include <iostream>
#include "erasure_online_codes_encoder.h"
#include "erasure_online_codes_decoder.h"
#include "test.h"

#define _CRTDBG_MAP_ALLOC
#include "stdlib.h"
#include "crtdbg.h"

using namespace std;

int main()
{
	int* data = new int[5000];
	for (int i = 0; i < 5000; i++)
		data[i] = i * 2;
	auto res1 = test_int_Ain_Aout_1(data,5000);
	cout << res1.data_len << " " << res1.error_num << " " << res1.received_len << endl;
	delete data;

	//_CrtMemCheckpoint(&s2);
	//if (_CrtMemDifference(&s3, &s1, &s2))
	//	_CrtMemDumpStatistics(&s3);
	_CrtDumpMemoryLeaks();
	getchar();
}