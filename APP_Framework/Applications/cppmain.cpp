/*
* Copyright (c) 2020 AIIT XUOS Lab
* XiUOS is licensed under Mulan PSL v2.
* You can use this software according to the terms and conditions of the Mulan PSL v2.
* You may obtain a copy of Mulan PSL v2 at:
*        http://license.coscl.org.cn/MulanPSL2
* THIS SOFTWARE IS PROVIDED ON AN "AS IS" BASIS, WITHOUT WARRANTIES OF ANY KIND,
* EITHER EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO NON-INFRINGEMENT,
* MERCHANTABILITY OR FIT FOR A PARTICULAR PURPOSE.
* See the Mulan PSL v2 for more details.
*/

#include <xiuos.h>
#include <stdio.h>
#include <cstdlib>
using namespace std;
extern "C" void KPrintf(const char *fmt, ...);

class Animal    //parent class

{

public:

    virtual void Eat()
    {

        KPrintf("eat\n");

    }

    void Sleep()
    {
        KPrintf("sleep\n");
    }


};

class Fish :public Animal    //subclass
{

public:

    void Eat()
    {
        KPrintf("fish eat\n");


    }
};

void Doeat(Animal& animal)
{
    animal.Eat();
}

void MemTest2()
{
    int i;
    char *ptr = NULL; /* memory pointer */

    for (i = 0; ; i++) {
        /* allocate (1<<i) bytes memory every single time */
        ptr = (char *)operator new(1 << i);

        /* if allocate successfully */
        if (ptr != NULL) {
            KPrintf("get memory :%d byte\n", (1 << i));
            /* release the memory */
            operator delete(ptr);
            KPrintf("free memory :%d byte\n", (1 << i));
            ptr = NULL;
        } else {
            KPrintf("try to get %d byte memory failed!\n", (1 << i));
            break;
            //return 0;
        }
    }

}

void OverLoadTest(int a)
{
    KPrintf("output is a int number: %d\n", a);
}

void OverLoadTestDouble(int a,int b )
{
    KPrintf("output is 2 int number: %d and %d\n", a,b);
}

template<typename T>
void MySwap(T& a, T& b)
{
    T temp = a;
    a = b;
    b = temp;
}

extern "C" int cppmain(void)
{
    MemTest2();

    class Fish fish;
    Doeat(fish);

    int a = 3;
    int b = 5;
    void OverLoadTestDouble(int a, int b);
    OverLoadTestDouble(a, b);
    MySwap(a,b);
    KPrintf("with template the output is: %d and %d\n", a,b);

    return 0;
}
