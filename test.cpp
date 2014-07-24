#include "erasure_online_codes_decoder.h"
#include "erasure_online_codes_encoder.h"
#include <time.h>
#include <iostream>
#include "test.h"
#include "rand.h"
#include "mtrand.h"


test_result test_int_Ain_Aout_1(int* data, int len)
{
	erasure_online_codes_encoder<int> enc(0.01, 0.005, 3,
		[](int& a, int &b, int *dest){*dest = a^b; return dest; },
		[](int n){return new int[n]; },
		[](int* data){delete data; });

	erasure_online_codes_decoder<int> dec(0.01, 0.005, 3,
		[](int& a, int &b, int *dest){*dest = a^b; return dest; },
		[](int n){return new int[n]; },
		[](int* data){delete data; });
	
	enc.input(data, len, 1);
	dec.start(len, 1);
	int i = 0;

	MTRand_int32 random(time(0));
	
	while (!(dec.is_finished()))
	{
		int degree;
		int* out=new int(0);

		int sd = random();
		//int sd = i * 2;
		enc.output(sd, &degree, out);
		dec.input(out, sd);
		i++;
	}
	int* dest = new int[len];
	dec.copy_to(dest);
	int err_num = 0;
	for (int i = 0; i < len; i++)
		if (dest[i] != data[i]){
		std::cout << "error occur: " << i << " " << dest[i] << " " << data[i] << std::endl;
		err_num++;
		}
	delete(dest);
	return test_result(len, i, err_num);
}