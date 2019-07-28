#include <stdio.h>
#include "skip_s.h"
#include "skip_r.h"
#include "runcheck.cpp"

int main(int argc, char **argv)
{
	sender::Msg M;
	M.set_f8_0(8);
	M.set_f8_1(88);
	runcheck(M);
	M.set_f16_0(16);
	M.set_f16_1(1616);
	runcheck(M);
	M.set_f32_0(32);
	M.set_f32_1(3232);
	runcheck(M);
	M.set_f64_0(64);
	M.set_f64_1(6464);
	runcheck(M);
	M.set_uvi_0(9999);
	M.set_uvi_1(99999999);
	runcheck(M);
	M.add_ru_0(0);
	M.add_ru_0(1000);
	M.add_ru_0(2);
	M.add_ru_0(3000);
	M.add_ru_1(10);
	M.add_ru_1(1001);
	M.add_ru_1(21);
	M.add_ru_1(3001);
	runcheck(M);
	M.set_s_0("string0");
	M.set_s_1("string1");
	runcheck(M);
	M.add_ri_0(0);
	M.add_ri_0(-1);
	M.add_ri_0(2);
	M.add_ri_0(-3);
	M.add_ri_1(1);
	M.add_ri_1(-111);
	M.add_ri_1(222);
	M.add_ri_1(-333);
	runcheck(M);
	M.add_rs_0(1111);
	M.add_rs_0(-1111);
	M.add_rs_0(2222);
	M.add_rs_0(-3333);
	M.add_rs_1(5555);
	M.add_rs_1(-5555);
	M.add_rs_1(66666);
	M.add_rs_1(-66666);
	runcheck(M);
	M.set_end("ende");
	runcheck(M);

	string binary;
	M.toString(binary);

	receiver::Msg RM;
	RM.fromMemory(binary.data(),binary.size());

	string back;
	RM.toString(back);

	sender::Msg SM;
	SM.fromMemory(back.data(),back.size());

	M.clear_f8_1();
	M.clear_f16_1();
	M.clear_f32_1();
	M.clear_f64_1();
	M.clear_uvi_1();
	M.clear_ru_1();
	M.clear_ri_1();
	M.clear_rs_1();
	M.clear_s_1();

	if (M != SM)
		fail("M==SM",&M,&SM);

	printf("%s: %s\n",argv[0],testcnt());
}
