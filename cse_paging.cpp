/*rrk310*/
//Assignment 3 - "Performance analysis of PFF and VSWS page replacement algorithms"
#include<iostream>
#include<vector>
#include<fstream>
#include<map>
#include<climits>

using namespace std;

class PFF //PageFault Frequency
{
	class Result //Holds Results for very 'F' run
	{
		public:
			int faults; //Track num page faults occuring after every memory reference
			int numOfNewFrames; //Track num of new frames alloc'ed
			int maxResidentSize;//Tracks how big the number of resident pages were at one point during simulation
			int lessThanTenFrames;//Tracks num of times , used frames are less than 10
			Result()
			{
				faults =0;
				numOfNewFrames =0;
				maxResidentSize=0;
				lessThanTenFrames=0;
			}
			void disp()
			{
				cout<<"max page faults "<<faults<<" max new frames alloc'ed "<<numOfNewFrames<<" maxResidentSize "<<maxResidentSize<<" lessThanTenFrames count "<<lessThanTenFrames<<endl;
			}
	};
	map<int, Result > recordedResults; //Record per F value
	void pffOneRun(Result &result ,vector<int> &references ,int fval ,int resSize)
	{
		int run=0;//virtual time
		int lastPageFault=run;
		int numOfPageFaults =0;
		int numOfNewFrames =0;
		int currResidentSize=0;
		int maxResidentSize =INT_MIN;
		int maxPages = resSize;//max pages that can be occupied by the process with use bit.
		map<int,bool> currentPages;
		//This map is dynamic , so if key contains - it means we have a frame available(it can be occupied or not) , 
		//if not then a new frame is like adding a <key,val> pair
		for(vector<int>::iterator iter = references.begin(); iter!=references.end();iter++)
		{
			run++;
			int markedPages = retCurrentMarkedPages(currentPages);
			if(markedPages <10)
				result.lessThanTenFrames++;
			if(markedPages == maxPages) //sanity check
			{
				cout<<"our fixed size of max pages for this process has exceeded so cant accomodate any more "<<markedPages<<endl;
				break;
			}	
			if(currentPages.find(*iter) != currentPages.end())
			{
				currentPages[*iter] = true;//mark active it means there is a frame as well
			}
			else
			{
				numOfPageFaults++;
				if(run - lastPageFault > fval)
				{
					//Now remove the free pages
					vector<int> toBeDeleted;
					for (map<int,bool>::iterator iter = currentPages.begin() ;iter!=currentPages.end(); iter++ )
					{
						if((*iter).second == false)//fetch pages not accessed
							toBeDeleted.push_back((*iter).first);
						else
							currentPages[(*iter).first] = false;//mark them inactive if there are active
					}
					for (vector<int>::iterator iter = toBeDeleted.begin() ;iter!=toBeDeleted.end() ; iter++)
					{
						currentPages.erase(*iter);  
						currResidentSize--;
					}
				}
				else
				{
					allocateNewFrame();
					numOfNewFrames++;
					currentPages[*iter] = true;//Since we have are using a dynamic array , no need to allocate new frame .Just make sure of sanity.
					currResidentSize++;
					if(maxResidentSize < currResidentSize)
						maxResidentSize = currResidentSize;
				}
				lastPageFault = run;//record this page fault
			}
		}
		result.faults = numOfPageFaults;
		result.numOfNewFrames = numOfNewFrames;
		result.maxResidentSize = maxResidentSize;
	}

	void allocateNewFrame(){} //dummy function to signify new frame allocation

	int retCurrentMarkedPages(map<int,bool> &currentPages)//returns pages in use.
	{
		int count =0;
		for(map<int,bool>::iterator iter = currentPages.begin(); iter!=currentPages.end();iter++)
		{
			if((*iter).second == true)
			{
				count++;
			}
		}
		return count;
	}
public:
	void startPFF(int resSize ,vector<int> &references)
	{
		for(int Fvalue = 1 ; Fvalue <=20 ; Fvalue++)//taking arbitrary numbers from 1 -20 just to prove how fvalue can reduce page faults.
		{
			Result result;
			pffOneRun(result,references,Fvalue,resSize);
			recordedResults.insert(make_pair<int, Result>(Fvalue,result));
		}
	}

	void displayResults()
	{
		int minPageFaults =INT_MAX;
		int minResidentSize =INT_MAX;
		int Fvalue = -1;
		int maxNewFrames = INT_MIN;
		for (map<int,Result>::iterator iter = recordedResults.begin() ;iter!=recordedResults.end(); iter++ )
		{
			cout<<"for Fvalue "<<(*iter).first<<endl;
			(*iter).second.disp();
			if((*iter).second.numOfNewFrames > maxNewFrames )
				maxNewFrames = (*iter).second.numOfNewFrames;
			if((*iter).second.faults < minPageFaults)
			{
				minPageFaults = (*iter).second.faults;
				Fvalue = (*iter).first;
				minResidentSize = (*iter).second.maxResidentSize;
			}
		}
		cout<<"optimal F value accross runs is Fvalue "<<Fvalue<<" page faults "<<minPageFaults<<" min resident size "<<minResidentSize<<endl;
		cout<<"the max new frames alloced amongst all runs is "<<maxNewFrames<<endl;
	}
};

class VSWS
{
public:
	class Sample
	{
	public:
		int M; //min length of sampling interval
		int L; //max length of sampling interval
		int Q; //num faults in one sampling instance
		Sample(int M,int L, int Q)
		{
			this->M = M;
			this->L = L;
			this->Q = Q;
		}
		void display()
		{
			cout<<"For Sample M "<<M<<" L "<<L<<" Q "<<Q<<endl;
		}
	};

	class SResult //Holds Results for very sampling run
	{
		public:
			int faults; //Track num page faults occuring after every memory reference
			int numOfNewFrames; //Track num of new frames alloc'ed
			int maxResidentSize;//Tracks how big the number of resident pages were at one point during simulation
			int lessThanTenFrames;//Tracks num of times , used frames are less than 10
			Sample &sample;//Holds a ref to sample data
			SResult(Sample &sample):sample(sample)
			{
				faults =0;
				numOfNewFrames =0;
				maxResidentSize=0;
				lessThanTenFrames=0;
			}
			void disp()
			{
				sample.display();
				cout<<"max page faults "<<faults<<" max new frames alloc'ed "<<numOfNewFrames<<" maxResidentSize "<<maxResidentSize<<" lessThanTenFrames count "<<lessThanTenFrames<<endl;
			}
	};

private:
	vector<SResult > recordedResults; //Record per F value
	vector<Sample > samples; //all samples

	void vswsOneRun(SResult &result ,vector<int> &references ,int resSize ,Sample &sample)
	{
		int M = sample.M;
		int L = sample.L;
		int Q = sample.Q;
		int currVirtualTime = 1;
		int numSamples =0 ;
		int maxPages = resSize;//max pages that can be occupied by the process with use bit.
		int numOfNewFrames =0;
		map<int,bool> currentPages; 
		//This map is dynamic , so if key contains - it means we have a frame available(it can be occupied or not) , 
		//if not then a new frame is like adding a <key,val> pair

		int minResidentSize = INT_MAX; //stores min residents for this run
		int maxResidentSize = INT_MIN;//stores max residents for this run
		int numFaults =0; //stores num of page faults encountered.
		int perSampleFault =0;
		int runTime = L;
		int evaluatedPerSampleWorkingSet=INT_MIN; //will store the max of all per samples , another approach can be to take mean.
		for(vector<int>::iterator iter = references.begin(); iter!=references.end();iter++)
		{
			if(currVirtualTime == 1)//begin interval
			{
				runTime = L;
				unMarkAllPages(currentPages);
			}
			if(currVirtualTime == runTime)//end interval - runTime can be bounded by M or L based on current Q
			{
				numSamples++;
				numFaults+=perSampleFault;
				currVirtualTime=1;
				perSampleFault=0;
				int currWorkingSet = retCurrentMarkedPages(currentPages);
				if(currWorkingSet >evaluatedPerSampleWorkingSet)
					evaluatedPerSampleWorkingSet = currWorkingSet;
				removeFreePages(currentPages);// release unreferenced pages
			}
			else
			{
				if(currentPages.find(*iter) != currentPages.end())
				{
					currentPages[*iter] = true;//mark active - it means there is a frame as well.
				}
				else
				{
					numOfNewFrames++; //Here no need to check if there is a frame available since its dynamic
					currentPages[*iter] = true; //Since we have are using a dynamic array , no need to allocate new frame .Just make sure of sanity.
					//page fault !
					perSampleFault++;
				}
				currVirtualTime++;
				int markedPages = retCurrentMarkedPages(currentPages);
				if(markedPages <10)
					result.lessThanTenFrames++;
				if(markedPages == maxPages) //sanity check
				{
					cout<<"our fixed size of max pages for this process has exceeded so cant accomodate any more "<<markedPages<<endl;
					break;
				}
				if(perSampleFault >= Q)
				{
					if(currVirtualTime < M)
					{
						runTime = M;
					}
					else
					{
						currVirtualTime = runTime; //will stop the current sample to determine use bits
					}
				}
			}
		}
		result.faults = numFaults;
		result.numOfNewFrames = numOfNewFrames;
		result.maxResidentSize = evaluatedPerSampleWorkingSet > maxResidentSize ? evaluatedPerSampleWorkingSet:maxResidentSize;
	}

	void unMarkAllPages(map<int,bool> &currentPages) //un marks all occupied pages 
	{
		for(map<int,bool>::iterator iter =currentPages.begin(); iter!=currentPages.end() ;iter++)
		{
			(*iter).second = false;
		}
	}

	void removeFreePages(map<int,bool> &currentPages) //removes pages that are free from resident set
	{
		vector<int> toBeDeleted;
		for (map<int,bool>::iterator iter = currentPages.begin() ;iter!=currentPages.end(); iter++ )
		{
			if((*iter).second == false)//fetch pages not accessed
				toBeDeleted.push_back((*iter).first);
		}
		for (vector<int>::iterator iter = toBeDeleted.begin() ;iter!=toBeDeleted.end() ; iter++)
		{
			currentPages.erase(*iter);  
		}
	}

	int retCurrentMarkedPages(map<int,bool> &currentPages) //returns pages in use.
	{
		int count =0;
		for(map<int,bool>::iterator iter = currentPages.begin(); iter!=currentPages.end();iter++)
		{
			if((*iter).second == true)
			{
				count++;
			}
		}
		return count;
	}

	void allocateNewFrame(){} //dummy function to signify new frame allocation
public:
	VSWS()
	{
		//run on these samples
		samples.push_back(Sample(1,4,1));
		samples.push_back(Sample(8,16,4));
		samples.push_back(Sample(8,20,6));
		samples.push_back(Sample(12,30,4));
	}

	void startVSWS(int resSize ,vector<int> &references)
	{
		for(vector<Sample>::iterator iter = samples.begin(); iter!=samples.end() ;iter++)
		{
			SResult result(*iter);
			vswsOneRun(result, references, resSize, *iter);
			recordedResults.push_back(result);
		}
	}

	void displayResults()
	{
		int minPageFaults =INT_MAX;
		int minResidentSize =INT_MAX;
		int maxNewFrames = INT_MIN;
		int index = -1;
		int found = -1;
		for (vector<SResult>::iterator iter = recordedResults.begin() ;iter!=recordedResults.end(); iter++ )
		{
			index++;
			(*iter).disp();
			if((*iter).numOfNewFrames > maxNewFrames )
				maxNewFrames = (*iter).numOfNewFrames;
			if((*iter).faults < minPageFaults)
			{
				minPageFaults = (*iter).faults;
				minResidentSize = (*iter).maxResidentSize;
				found = index;
			}
		}
		cout<<"optimal values are page faults "<<minPageFaults<<" min resident size "<<minResidentSize<<" for below sample "<<endl;
		if(found !=-1) recordedResults[found].sample.display();
		cout<<"the max new frames alloced amongst all runs is "<<maxNewFrames<<endl;
	}
};

int startSimulation(string &fileName)
{
	vector<int> pageReferences;
	int currResidentSize;
	int ret =0;
	ifstream memFile(fileName);
	if(!memFile)
	{
		ret = -1;
		cout<<"fopen failed!"<<endl;
	}
	else
	{
		string currSize;
		string references;
		int numReferences=0;
		getline(memFile,currSize);
		currResidentSize = atoi(currSize.c_str());
		cout<<"resident size - "<<currResidentSize<<endl;
		while(getline(memFile,references))
		{
			pageReferences.push_back(atoi(references.c_str()));
			numReferences++;
		}
		cout<<"num memory refs - "<<numReferences<<endl;
	
		cout<<"*********** begin PFF simulation ***********"<<endl;
		PFF pff;
		pff.startPFF(currResidentSize, pageReferences);
		pff.displayResults();
		//After finding the results on a run of 129 page references , it was found that optimal value for f{1-20} was 12 
		//and the number page faults were 42 as opposed to 60 with f value =1
		//also there was a considerable improvement with resident size of 38 there by proving that the page fault rate decreases 
		//as the resident set increases.
		//In the dataset after introducing page references of different locality , it was found that the max resident size increased for a while.
		cout<<"*********** end PFF simulation **************"<<endl;


		cout<<"*********** begin VSWS simulation ***********"<<endl;
		VSWS vsws;
		vsws.startVSWS(currResidentSize, pageReferences);
		vsws.displayResults();
		//After running the VSWS on the same dataset of 129 page references, it was found that a better minimum fault rate of 39 was acheived
		//for this sample M 12 L 30 Q 4 and a better resident size of 11.
		//Based on the max resident size , its clear that even with a less resident page , lesser fault rate can be acheived.
		//The algorithm was also run on page references of different locality and produces more or less same result.
		//On comparison it can be seen VSWS algorithm produces much lesser fault rate on an average provided a good sample is provided for that instant.
		cout<<"************ end VSWS simulation ***************"<<endl;
	}
	return ret;
}

int main(int argc ,char **argv)
{
	string fileName;
	int ret =0;
	cout<<"Enter file name : "<<endl;
	cin>>fileName;
	cout<<"File name - "<<fileName<<endl;
	ret = startSimulation(fileName);
	if(ret != 0)
		cout<<"simulation error!"<<endl;
	return 0;
}