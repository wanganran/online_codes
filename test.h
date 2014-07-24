#ifndef TEST_H
#define  TEST_H
struct test_result
{
	int data_len;
	int received_len;
	int error_num;

	test_result(int data_len, int received_len, int error_num) {
		this->data_len = data_len;
		this->received_len = received_len;
		this->error_num = error_num;
	}
};

test_result test_int_Ain_Aout_1(int* data,int len);

#endif