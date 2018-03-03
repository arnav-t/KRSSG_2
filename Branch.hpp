#define Branch_HPP

class Branch
{
	private:
		cv::Point location;
		Branch *parent;
		std::vector<Branch *> children;
	public:
		Branch(cv::Point l)
		{
			location = l;
			parent = NULL;
		}
		void setParent(Branch *p)
		{
			parent = p;
		}
		cv::Point getLocation()
		{
			return location;
		}
		Branch *addChild(cv::Point l)
		{
			cv::Point lAdd;
			if(abs(location.y - l.y) + abs(location.x - l.x) <= stepThreshold)
				lAdd = l;
			else
			{
				float mag = sqrt(pow((l.x - location.x),2) + pow((l.y - location.y),2));
				lAdd.x = location.x + (l.x - location.x)*stepThreshold/mag;
				lAdd.y = location.y + (l.y - location.y)*stepThreshold/mag;
			}
			if(imgg.at<uchar>(lAdd.y,lAdd.x) < 128)
			{
				Branch *newChild = new Branch(lAdd);
				newChild->setParent(this);
				children.push_back(newChild);
				return newChild;
			}
			return NULL;
		}
		Branch *getClosest(cv::Point l)
		{
			Branch *closest = this;
			int minDist = abs(location.y - l.y) + abs(location.x - l.x);
			for(int i=0;i<children.size();++i)
			{
				Branch *closestChild = children[i]->getClosest(l);
				cv::Point loc = closestChild->getLocation();
				int childMinDist = abs(loc.y - l.y) + abs(loc.x - l.x);
				if(childMinDist < minDist)
				{
					closest = closestChild;
					minDist = childMinDist;
				}
			}
			return closest;
		}
		Branch *getClosestToBranch(Branch *b)
		{
			Branch *closest = this;
			cv::Point l = b->getLocation();
			int minDist = abs(location.y - l.y) + abs(location.x - l.x);
			for(int i=0;i<children.size();++i)
			{
				Branch *closestChild = children[i]->getClosest(l);
				cv::Point loc = closestChild->getLocation();
				int childMinDist = abs(loc.y - l.y) + abs(loc.x - l.x);
				if(childMinDist < minDist)
				{
					closest = closestChild;
					minDist = childMinDist;
				}
			}
			if(minDist <= stepThreshold)
				return closest;
			else 
				return NULL;
		}
		void drawBranch(int id)
		{
			for(int i=0;i<children.size();++i)
			{
				if(id)
					line(img, location, children[i]->getLocation(), cv::Scalar(30,0,80), 1, CV_AA);
				else
					line(img, location, children[i]->getLocation(), cv::Scalar(20,50,0), 1, CV_AA);
				children[i]->drawBranch(id);
			}
			//circle(img, location, 1, cv::Scalar(0,25,25),CV_FILLED);
		}
		void traceParent()
		{
			if(parent != NULL)
			{
				line(img, location, parent->getLocation(), cv::Scalar(0,0,255), 1, CV_AA);
				parent->traceParent();
			}
		}
};