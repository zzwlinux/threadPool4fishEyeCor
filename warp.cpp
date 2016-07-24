#include <opencv2/opencv.hpp>
#include <cmath>
#include <iostream>
#include <stdio.h>
#include <string.h>
#include <unistd.h>
#include <sys/time.h>

#include "ThreadPool.h"

struct interpolation_table
{
public:
    interpolation_table(int x = 0,int y = 0,float p0 = 0.0f,float p1 = 0.0f)
        :xBegin(x),yBegin(y),xProportion(p0),yProportion(p1)
    {}
    interpolation_table(const interpolation_table& ref)
    {
        xBegin        = ref.xBegin;
        yBegin        = ref.yBegin;
        xProportion   = ref.xProportion;
        yProportion   = ref.yProportion;
    }
    int   xBegin,yBegin;
    float xProportion;
    float yProportion;
};

std::vector<interpolation_table> frontTable;
std::vector<interpolation_table> leftTable;
std::vector<interpolation_table> behindTable;
std::vector<interpolation_table> rightTable;


inline cv::Vec3b bilinear_interpolation(const interpolation_table& table,const cv::Mat& input)
{
    const cv::Vec3b& lu = input.at<cv::Vec3b>(table.yBegin,table.xBegin);
    const cv::Vec3b& ru = input.at<cv::Vec3b>(table.yBegin+1,table.xBegin);
    const cv::Vec3b& lb = input.at<cv::Vec3b>(table.yBegin,table.xBegin+ 1);
    const cv::Vec3b& rb = input.at<cv::Vec3b>(table.yBegin+1,table.xBegin+ 1);

    // X direction interpolation
    cv::Vec3b temp0  = lu*(1 - table.xProportion) + ru* table.xProportion;
    cv::Vec3b temp1  = lb*(1 - table.xProportion) + rb* table.xProportion;
    // Y direction interpolation
    cv::Vec3b result = temp0*(1 - table.yProportion) + temp1* table.yProportion;
    return result;
}

enum direct{
    none   = -1,
    d_front  = 0,
    d_left   = 1,
    d_behind = 2,
    d_right  = 3
};

struct Data {
    cv::Mat               input;
    std::vector<cv::Mat>  outPut;

    Data()
    {
        outPut.clear();
        cv::Mat left   (340,1068,CV_8UC3,cv::Scalar(0,0,0)); outPut.push_back(left);
        cv::Mat right  (340,1068,CV_8UC3,cv::Scalar(0,0,0)); outPut.push_back(right);
        cv::Mat front  (340,1068,CV_8UC3,cv::Scalar(0,0,0)); outPut.push_back(front);
        cv::Mat behind (340,1068,CV_8UC3,cv::Scalar(0,0,0)); outPut.push_back(behind);
    }
    ~Data() {}

    Data(Data& data) {
        input  = data.input;
        outPut = data.outPut;
    }

    const Data& operator= (Data &data) {
        input  = data.input;
        outPut = data.outPut;
        return *this;
    }
};

////////////////////////////////////////
/// \brief look up table
/// front
void warp2ImagesTablefront(Data &data)
{
    std::vector<interpolation_table>::iterator iter = frontTable.begin();
    for(int row = 0; row < 340; row++){
        for(int col = 0; col < 1068; col++)
        {
            data.outPut[d_front].at<cv::Vec3b>(row,col) = bilinear_interpolation(*iter++,data.input);
        }
    }
}
/// left
void warp2ImagesTableleft(Data &data)
{
    std::vector<interpolation_table>::iterator iter = leftTable.begin();
    for(int row = 0; row < 340; row++){
        for(int col = 0; col < 1068; col++)
        {
            data.outPut[d_left].at<cv::Vec3b>(row,col) = bilinear_interpolation(*iter++,data.input);
        }
    }
}
/// behind
void warp2ImagesTablebehind(Data &data)
{
    std::vector<interpolation_table>::iterator iter = behindTable.begin();
    for(int row = 0; row < 340; row++){
        for(int col = 0; col < 1068; col++)
        {
            data.outPut[d_behind].at<cv::Vec3b>(row,col) = bilinear_interpolation(*iter++,data.input);
        }
    }
}
/// right
void warp2ImagesTableright(Data &data)
{
    std::vector<interpolation_table>::iterator iter = rightTable.begin();
    for(int row = 0; row < 340; row++){
        for(int col = 0; col < 1068; col++)
        {
            data.outPut[d_right].at<cv::Vec3b>(row,col) = bilinear_interpolation(*iter++,data.input);
        }
    }
}


void initTable(void)
{
    cv::Point2i center(978,704);
    int rad = 680;
    leftTable.resize(340*1068);          leftTable.clear();
    rightTable.resize(340*1068);         rightTable.clear();
    frontTable.resize(340*1068);         frontTable.clear();
    behindTable.resize(340*1068);        behindTable.clear();

    ////////////////////////////////////////
    /// \brief creat table
    ///
    /// front
    for(int row = 0; row < 340; row++){ // l
        int col = 0;
        for(float theta = 5*M_PI/4 ;theta < 7*M_PI/4, col < 1068; theta += float(1.0/rad),col++)
        {
            double temp_clos = double((rad - row) * cos(theta));
            double temp_rows = double((rad - row) * sin(theta));
            int tempX = floor(temp_clos+center.x);
            int tempY = floor(temp_rows+center.y);
            float p0 = temp_clos+center.x - tempX;
            float p1 = temp_rows+center.y - tempY;
            frontTable.push_back(interpolation_table(tempX,tempY,p0,p1));
        }
    }
    ////////////////////////////////////////
    /// left

    for(int row = 0; row < 340; row++){
        int col = 0;
        for(float theta = 3*M_PI/4 ;theta < 5*M_PI/4, col < 1068; theta += float(1.0/rad),col++)
        {
            double temp_clos = double((rad - row) * cos(theta));
            double temp_rows = double((rad - row) * sin(theta));
            int tempX = floor(temp_clos+center.x);
            int tempY = floor(temp_rows+center.y);
            float p0 = temp_clos+center.x - tempX;
            float p1 = temp_rows+center.y - tempY;
            interpolation_table tempTable(tempX,tempY,p0,p1);
            leftTable.push_back(tempTable);
        }
    }
    ////////////////////////////////////////
    /// behind

    for(int row = 0; row < 340; row++){
        int col = 0;
        for(float theta = 1*M_PI/4 ;theta < 3*M_PI/4, col < 1068; theta += float(1.0/rad),col++) //theta
        {
            double temp_clos = double((rad - row) * cos(theta));
            double temp_rows = double((rad - row) * sin(theta));
            int tempX = floor(temp_clos+center.x);
            int tempY = floor(temp_rows+center.y);
            float p0 = temp_clos+center.x - tempX;
            float p1 = temp_rows+center.y - tempY;
            interpolation_table tempTable(tempX,tempY,p0,p1);
            behindTable.push_back(tempTable);
        }
    }

    ////////////////////////////////////////
    /// right

    for(int row = 0; row < 340; row++){
        int col = 0;
        for(float theta = -1*M_PI/4 ;theta < 1*M_PI/4, col < 1068; theta += float(1.0/rad),col++) //theta
        {
            double temp_clos = double((rad - row) * cos(theta));
            double temp_rows = double((rad - row) * sin(theta));
            int tempX = floor(temp_clos+center.x);
            int tempY = floor(temp_rows+center.y);
            float p0 = temp_clos+center.x - tempX;
            float p1 = temp_rows+center.y - tempY;
            interpolation_table tempTable(tempX,tempY,p0,p1);
            rightTable.push_back(tempTable);
        }
    }
}

void warpAPI()
{
    initTable();
    ThreadPool<Data> thread(4);
    Data data;

    cv::VideoCapture cap("data/flight1_01.MP4");
    uint64 count = 0;
    for (int j = 0; j < 200; ++j) {
        cap >> data.input;
        struct timeval	begin_,end_;
        gettimeofday(&begin_,0);
        int i = 0;
        thread.reduce(i++, warp2ImagesTablefront, data);
        thread.reduce(i++, warp2ImagesTableleft, data);
        thread.reduce(i++, warp2ImagesTablebehind, data);
        thread.reduce(i,   warp2ImagesTableright, data);

        while(!thread.isSynchronize());
        gettimeofday(&end_,0);
        count += (end_.tv_sec - begin_.tv_sec)*1000000 + (end_.tv_usec - begin_.tv_usec);
    }

    printf("total time = %ld! \n",count);
}

int main(int argc, char *argv[])
{
    warpAPI();
    return 0;
}
