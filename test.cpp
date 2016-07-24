#include <stdio.h>
#include <string.h>
#include <unistd.h>

#include "ThreadPool.h"

#include <opencv2/opencv.hpp>

void testPrint(int x) {
    printf("thread = %d\n", x);
}


struct Data {
    cv::Mat input;
    cv::Mat *outPut;
    Data() {}
    ~Data() {}
    
    Data(Data& data) {
        input = data.input;
        outPut = data.outPut;
    }

    const Data& operator= (Data &data) {
        input = data.input;
        //cv::imshow("show", input);
        outPut = data.outPut;

        return *this;
    }
};


void testOpt(Data &data)
{
    (*data.outPut) = data.input.clone();
}

void testUnpoint() {
    ThreadPool<int> threads(10);
    int cnt = 0;
    while (cnt < 10) {
        for(int i = 0; i < 10; ++i) {
            threads.reduce(i, testPrint, i);
        }
        cnt++;
        while(!threads.isSynchronize());
    }



    printf("test unpointer version over!\nplease press any key to exit..");
    getchar();

    exit(0);
}

void testPoint() {
    int idata[100];
    for(int i = 0; i < 100; ++i)
    {
        idata[i] = i;
    }

    ThreadPool<int *> threads(10);
    int cnt = 0;
    while (cnt < 10) {
        for(int i = 0; i < 10; ++i) {
            threads.reduce(i, testPrint, idata + 10 * i);
        }
        cnt++;
        while(!threads.isSynchronize());
    }

    printf("test pointer version over!\nplease press any key to continue...");
    getchar();

    Data data[4];

    for(int i = 0; i < 4; ++i) {
        char buff[32];
        memset(buff, 0, 32);
        sprintf(buff, "data/%d.jpg", i + 1);
        data[i].input = cv::imread(cv::String(buff));
        data[i].outPut = new cv::Mat;
    }

    ThreadPool<Data*> thread(4);

    for(int i = 0; i < 4; ++i) {
        thread.reduce(i, testOpt, data);
    }

    while(!thread.isSynchronize());

    for(int i = 0; i < 4; ++i) {
        char buff[32];
        sprintf(buff, "%d.jpg", i + 1);
        cv::imshow(buff, *(data[i].outPut));
        cv::waitKey(0);
    }

    printf("test pointer version over!\nplease press any key to exit..");
    getchar();

    exit(0);
}

int main0(int argc, char *argv[]) {
    if(argc == 1) {
        printf("default test unpointer version!\n");
        testUnpoint();
    }

    else {
        if(!strcmp(argv[1], "testPointer")) {
            printf("choose pointer version!\n");
            testPoint();
        }
        else if(!strcmp(argv[1], "testUnPointer")) {
            printf("choose unpointer version\n");
            testUnpoint();
        }
        else {
            printf("err!! please input \"testPointer\" or \"testUnPointer\" to test this code!\n");
            return 0;
        }
    }

    return 0;
}
