#include "opencv2/highgui/highgui.hpp"
#include "opencv2/imgproc/imgproc.hpp"
#include "opencv2/core/core.hpp"
#include <vector>
#include <cmath>
#include <cstdlib>
#include <iostream>
#include <pthread.h>

#define SIZE 512

const int stepThreshold = 4;
const int traceThreshold = 10;
const int maxNodes = 5000;

cv::Mat img(SIZE,SIZE,CV_8UC3,cv::Scalar(0,0,0));
cv::Mat imgg(SIZE,SIZE,CV_8UC1,cv::Scalar(0));
int state = 0;
cv::Point start, finish, current, previous;
bool drawing = false;

#ifndef Branch_HPP
#include "Branch.hpp"
#endif

using namespace cv;
using namespace std;

void *createTree(void *);

struct threadData
{
	int threadID;
	Point startPoint;
	threadData(int tID, Point sPoint)
	{
		threadID = tID;
		startPoint = sPoint;
	}
};

pthread_t threads[2];
Branch *trees[2];

void init(int event, int x, int y, int flags, void* a)
{
	current = Point(x,y);
	if(event == EVENT_RBUTTONDOWN)
	{
		if(state == 0)
		{
			state += 1;
			start = Point(x,y);
			circle(img, start, 3, Scalar(0,255,0),CV_FILLED);
			imshow("RRT",img);
			cout << "Start point registered at (" << y  << ", " << x << ")\n";
			waitKey(1);
		}
		else if(state == 1)
		{
			state += 1;
			finish = Point(x,y);
			circle(img, finish, 3, Scalar(0,0,255),CV_FILLED);
			imshow("RRT",img);
			cout << "Finish point registered at (" << y  << ", " << x << ")\n";
			waitKey(1);
			cout << "Creating trees...\n";
			threadData *startData = new threadData(0, start);
			threadData *finishData = new threadData(1, finish);
			if(pthread_create(&threads[0], NULL, createTree, (void *)startData))
				cout << "Error in start thread!\n";
			if(pthread_create(&threads[1], NULL, createTree, (void *)finishData))
				cout << "Error in finish thread!\n";
		}
	}
	else if(event == EVENT_LBUTTONDOWN)
	{
		drawing = true;
		previous = current;
	}
	else if(event == EVENT_LBUTTONUP)
		drawing = false;
	if(drawing)
	{
		line(img, previous, current, Scalar(255,255,255), 3, CV_AA);
		line(imgg, previous, current, 255, 3, CV_AA);
		previous = current;
		imshow("RRT",img);
		waitKey(1);
	}
	
}


void *createTree(void *data)
{
	if(state == 3)
	{
		cout << "Thread finished.\n"; 
		pthread_exit(NULL);
		return NULL;
			
	}
	threadData *tData = (threadData *)data;
	int id = tData->threadID;
	Point startPoint = tData->startPoint;
	trees[id] = new Branch(startPoint); 
	for(int j=0;j<maxNodes && state != 3;++j)
	{
		Branch *child = NULL;
		while(child == NULL)
		{
			Point randomPoint(rand()%SIZE,rand()%SIZE);
			Branch *closestNode = trees[id]->getClosest(randomPoint);
			child = closestNode->addChild(randomPoint);
		}
		if(id)
		{
			Branch *bridgePoint = trees[0]->getClosestToBranch(child);
			if(bridgePoint != NULL)
			{
				state += 1;
				img = Scalar(0,0,0);
				trees[id]->drawBranch(id);
				trees[0]->drawBranch(0);
				child->traceParent();
				bridgePoint->traceParent();
				line(img, child->getLocation(), bridgePoint->getLocation(), cv::Scalar(0,0,255), 1, CV_AA);
				circle(img, start, 3, Scalar(0,255,0),CV_FILLED);
				circle(img, finish, 3, Scalar(0,0,255),CV_FILLED);
				for(int y=0;y<SIZE;++y)
						for(int x=0;x<SIZE;++x)
							if(imgg.at<uchar>(y,x) == 255)
								img.at<Vec3b>(y,x) = Vec3b(255,255,255);
				imshow("RRT",img);
				waitKey(1);
				break;
			}
		}
		if(j%500 == 0)
		{
			if(id)
			{
				img = Scalar(0,0,0);
				trees[id]->drawBranch(id);
				trees[0]->drawBranch(0);
				circle(img, start, 3, Scalar(0,255,0),CV_FILLED);
				circle(img, finish, 3, Scalar(0,0,255),CV_FILLED);
				for(int y=0;y<SIZE;++y)
						for(int x=0;x<SIZE;++x)
							if(imgg.at<uchar>(y,x) == 255)
								img.at<Vec3b>(y,x) = Vec3b(255,255,255);
				imshow("RRT",img);
				waitKey(1);
			}
		}
	}
	/*Branch *lastNode = trees[id]->getClosest(finish);
	Point lastLoc = lastNode->getLocation();
	if(abs(lastLoc.y - finish.y) + abs(lastLoc.x - finish.x) <= traceThreshold)
			//lastNode->traceParent();*
	imshow("RRT",img);*/
	cout << "Thread finished.\n"; 
	//waitKey(1);
	pthread_exit(NULL);
}

int main()
{
	srand(time(NULL));
	namedWindow("RRT",CV_WINDOW_AUTOSIZE);
	imshow("RRT",img);
	waitKey(1);
	setMouseCallback("RRT", init, NULL);
	waitKey(0);
	//pthread_exit(NULL);
	return 0;
}