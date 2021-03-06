#include <iostream>
#include <fstream>
#include <sstream>
#include <string>
#include <cmath>
#include <cfloat>
#include <vector>
#include <cstdlib>
#include "DataSet.hpp"
#include "DataPrep.hpp"

using namespace std;
using std::vector;

template<class T>
bool inThere(const vector<T> &vec, T const &val)
{
	for (auto i : vec)
	{
		if (val == i)
			return true;
	}
	return false;
}

int main(int argc, char* argv[])
{
    string trainFile(argv[1]);
    string testFile(argv[2]);
    bool isTAN = argv[3][0] == 't' || argv[3][0] == 'T';
	
	dataPrep(trainFile, testFile);

	ifstream fin1, fin2;
	fin1.open("attr.tmp", ios::in);
	int attrCount, trainEntryCount;
	fin1 >> attrCount >> trainEntryCount;

	AbsAttr **initAttrList = new AbsAttr*[attrCount];
	string str, name, type;
	stringstream ss;
	getline(fin1, str);
	for (int i = 1; i <= attrCount; i++)
	{
		getline(fin1, str);
		ss.clear();
		ss.str(str);
		ss >> name >> type;
		string entryDir = "attrTrain/";
		entryDir = entryDir.append(to_string(i));
		fin2.open(entryDir);
		if (type == "nominal")
		{
			int valCount;
			istringstream ss2(str);
			for (valCount = -2; getline(ss2, str, ' '); valCount++)
				;
			initAttrList[i - 1] = new NominalAttr(name, valCount, ss, trainEntryCount, fin2);
		}
		else
		{
			initAttrList[i - 1] = new NumericalAttr(name, trainEntryCount, fin2);
		}
		fin2.close();
	}
	fin1.close();
    DataSet *dataSet = new DataSet(attrCount, initAttrList);

	AbsAttr **testAttrList = new AbsAttr*[attrCount];
	int testEntryCount;
	fin1.open("attr2.tmp", ios::in);
	fin1 >> attrCount >> testEntryCount;
	str.clear();
	name.clear();
	type.clear();
	getline(fin1, str);
	for (int i = 1; i <= attrCount; i++)
	{
		getline(fin1, str);
		ss.clear();
		ss.str(str);
		ss >> name >> type;
		string entryDir = "attrTest/";
		entryDir = entryDir.append(to_string(i));
		fin2.open(entryDir);
		if (type == "nominal")
		{
			int valCount;
			istringstream ss2(str);
			for (valCount = -2; getline(ss2, str, ' '); valCount++)
				;
			testAttrList[i - 1] = new NominalAttr(name, valCount, ss, testEntryCount, fin2);
		}
		else
		{
			testAttrList[i - 1] = new NumericalAttr(name, testEntryCount, fin2);
		}
		fin2.close();
	}
	fin1.close();
    DataSet *testDataSet = new DataSet(attrCount, testAttrList);
   
    if (!argv[4])
        system("rm -rf temp.sh attr.tmp attr2.tmp data.tmp data2.tmp attrTest attrTrain");

	ofstream fout;
	fout.precision(12);

	fout.open("result.dat", ios::out);
    if (isTAN)
    {
		// TAN NB Algo
		NominalAttr &targetAttr = *((NominalAttr*)dataSet->_attrList[attrCount-1]);
		int targetValCount = targetAttr.getPossibleValCount();
		int featureCount = attrCount-1;

		double *****counter = new double****[featureCount-1];
		double *****jointProb = new double****[featureCount - 1];
		
		double ***condProb = new double **[featureCount];
		for (int i = 0; i < featureCount; ++i)
		{
			int featValCount = ((NominalAttr*)dataSet->_attrList[i])->getPossibleValCount();
			condProb[i] = new double* [featValCount];
			for (int j = 0; j < featValCount; ++j)
			{
				condProb[i][j] = new double [targetValCount];
				// Calculate conditional probabilities of features wrt target
                for (int k = 0; k < targetValCount; ++k)
                {
					condProb[i][j][k] = 0.;
				}
			}
		}

		for (int i = 0; i < featureCount-1; i++)
		{
			NominalAttr & feat1 = *((NominalAttr*)dataSet->_attrList[i]);
			counter[i] = new double***[featureCount - i - 1];
			jointProb[i] = new double***[featureCount - i - 1];
			for (int j = 0; j < featureCount - i - 1; j++)
			{
				int feat2Index = i + j + 1;
				NominalAttr &feat2 = *((NominalAttr*)dataSet->_attrList[feat2Index]);
				int feat1ValCount = feat1.getPossibleValCount();
				int feat2ValCount = feat2.getPossibleValCount();
				counter[i][j] = new double **[feat1ValCount];
				jointProb[i][j] = new double **[feat1ValCount];
				for(int k = 0; k < feat1ValCount; k++)
				{
					counter[i][j][k] = new double* [feat2ValCount];
					jointProb[i][j][k] = new double* [feat2ValCount];
					for (int l = 0; l < feat2ValCount; l++)
					{
						counter[i][j][k][l] = new double [targetValCount];
						jointProb[i][j][k][l] = new double [targetValCount];
						for (int m = 0; m < targetValCount; m++)
						{
							counter[i][j][k][l][m] = jointProb[i][j][k][l][m] = 0.;
						}
					}
				}
			}
		}

		int* targetCounter = new int[targetValCount];
		for (int i = 0; i < targetValCount; ++i)
			targetCounter[i] = 0;
		
		for (int i = 0; i < trainEntryCount; ++i)
		{
			++targetCounter[targetAttr.getEntryValLabel(i)];
			for (int j = 0; j < featureCount - 1; ++j)
			{
				NominalAttr &feat1 = *((NominalAttr*)dataSet->_attrList[j]);
				++condProb[j][feat1.getEntryValLabel(i)][targetAttr.getEntryValLabel(i)];
				for (int k = 0; k < featureCount - j - 1; k++)
				{
					int feat2Index = j + k + 1;
					NominalAttr &feat2 = *((NominalAttr*)dataSet->_attrList[feat2Index]);
					counter[j][k][feat1.getEntryValLabel(i)][feat2.getEntryValLabel(i)]
						[targetAttr.getEntryValLabel(i)] += 1.;
				}
			}
			++condProb[featureCount-1][((NominalAttr*)dataSet->_attrList[featureCount-1])->getEntryValLabel(i)]\
				[targetAttr.getEntryValLabel(i)];
		}

		double **condMutualInfo = new double *[featureCount - 1];
		for (int i = 0; i < featureCount - 1; ++i)
		{
			condMutualInfo[i] = new double[featureCount - i - 1];
			for (int j = 0; j < featureCount - i - 1; ++j)
				condMutualInfo[i][j] = 0;
		}

		// Calculate conditional probabilities with Laplace estimator, and then calculate conditional mutual information
		for (int i = 0; i < featureCount; ++i)
		{
			NominalAttr &feat1 = *(NominalAttr*) dataSet->_attrList[i];
			int feat1ValCount = feat1.getPossibleValCount();
			for (int j = 0; j < feat1ValCount; ++j)
			{
				for (int k = 0; k < targetValCount; ++k)
				{
					condProb[i][j][k] = (condProb[i][j][k] + 1.) / (targetCounter[k] + feat1ValCount);
					if ( i != featureCount - 1)
					{
						for (int l = 0; l < featureCount - i - 1; ++l)
						{
							int feat2Index = i + l + 1;
							NominalAttr &feat2 = *(NominalAttr*)dataSet->_attrList[feat2Index];
							int feat2ValCount = feat2.getPossibleValCount();
							for (int m = 0; m < feat2ValCount; ++m)
								jointProb[i][l][j][m][k] = (counter[i][l][j][m][k] + 1) \
									/ (targetCounter[k] + feat1ValCount * feat2ValCount);
						}
					}
				}
			}
		}
		// calculate mutual conditional information
		for (int i = 0; i < featureCount - 1; i++)
		{
			NominalAttr &feat1 = *(NominalAttr*)dataSet->_attrList[i];
			int feat1ValCount = feat1.getPossibleValCount();
			for (int j = 0; j < featureCount - i - 1; ++j)
			{
				int feat2Index = i + j + 1;
				NominalAttr &feat2 = *(NominalAttr*) dataSet->_attrList[feat2Index];
				int feat2ValCount = feat2.getPossibleValCount();
				for (int k = 0; k < feat1ValCount; ++k)
				{
					for (int l = 0; l < feat2ValCount; ++l)
					{
						for (int m = 0; m < targetValCount; ++m)
						{
							condMutualInfo[i][j] += (counter[i][j][k][l][m] + 1)
								/ (trainEntryCount + feat1ValCount * feat2ValCount * targetValCount)
								* log2(jointProb[i][j][k][l][m] / condProb[i][k][m] / condProb[feat2Index][l][m]);
						}
					}
				}
			}
		}
		
		for (int i = 0; i < featureCount-1; i++)
		{
			NominalAttr & feat1 = *((NominalAttr*)dataSet->_attrList[i]);
			for (int j = 0; j < featureCount - i - 1; j++)
			{
				int feat2Index = i + j + 1;
				NominalAttr &feat2 = *((NominalAttr*)dataSet->_attrList[feat2Index]);
				int feat1ValCount = feat1.getPossibleValCount();
				int feat2ValCount = feat2.getPossibleValCount();
				for(int k = 0; k < feat1ValCount; k++)
				{
					for (int l = 0; l < feat2ValCount; l++)
					{
						delete[] jointProb[i][j][k][l];
					}
					delete[] jointProb[i][j][k];
				}
				delete[] jointProb[i][j];
			}
			delete[] jointProb[i];
 		}
		delete[] jointProb;
		jointProb = nullptr;

		int *findDad = new int[featureCount];
		for (int i = 0; i < featureCount; i++)
			findDad[i] = -1;

		vector<int> inTree = {0};
		// Build Maximum Spanning Tree
		while(inTree.size() < (decltype(inTree.size())) featureCount)
		{
			int bestDad, bestChild;
			double bestWeight;
			bool weightInitialised = false;
			for (auto i : inTree)
			{
				for (int j = 0; j < featureCount; j++)
				{
					if (inThere(inTree, j))
						continue;
					if (!weightInitialised)
					{
						bestWeight = i < j ? condMutualInfo[i][j-i-1] : condMutualInfo[j][i-j-1];
						bestChild = j;
						bestDad = i;
						weightInitialised = true;
					}
					else 
					{
						const double &currentWeight = i < j ? condMutualInfo[i][j-i-1] : condMutualInfo[j][i-j-1];
						if (currentWeight > bestWeight)
						{
							bestDad = i;
							bestChild = j;
							bestWeight = currentWeight;
						}
					}
				}
			}
			findDad[bestChild] = bestDad;
			inTree.push_back(bestChild);
		}


		double ****likelihood = new double ***[featureCount];
		// the first feature is the root of the max spanning tree, thus the iteration starts at 1
		for (int i = 1; i < featureCount; i++)
		{
			NominalAttr &currentFeat = *(NominalAttr*) dataSet->_attrList[i];
			int currentValCount = currentFeat.getPossibleValCount();
			likelihood[i] = new double **[currentValCount];
			int dadIndex = findDad[i];
			NominalAttr &dad = *(NominalAttr*)dataSet->_attrList[dadIndex];
			int dadValCount = dad.getPossibleValCount();
			for (int j = 0; j < currentValCount; j++)
			{
				likelihood[i][j] = new double *[dadValCount];
				for (int k = 0; k < dadValCount; k++)
				{
					likelihood[i][j][k] = new double [targetValCount];
					for (int l = 0; l < targetValCount; ++l)
					{
						double denominator = 0.;
						for (int m = 0; m < currentValCount; ++m)
						{
							denominator += i < dadIndex ? 
								counter[i][dadIndex - i - 1][m][k][l] :
								counter[dadIndex][i - dadIndex - 1][k][m][l];
						}
						likelihood[i][j][k][l] = i < dadIndex ? 
							(counter[i][dadIndex - i - 1][j][k][l] + 1) / (denominator + currentValCount) :
							(counter[dadIndex][i - dadIndex - 1][k][j][l] + 1) / (denominator + currentValCount);
					}
				}
			}
		}
		
		for (int i = 0; i < featureCount - 1; i++)
		{
			NominalAttr & feat1 = *((NominalAttr*)dataSet->_attrList[i]);
			for (int j = 0; j < featureCount - i - 1; j++)
			{
				int feat2Index = i + j + 1;
				NominalAttr &feat2 = *((NominalAttr*)dataSet->_attrList[feat2Index]);
				int feat1ValCount = feat1.getPossibleValCount();
				int feat2ValCount = feat2.getPossibleValCount();
				for(int k = 0; k < feat1ValCount; k++)
				{
					for (int l = 0; l < feat2ValCount; l++)
					{
						delete[] counter[i][j][k][l];
					}
					delete[] counter[i][j][k];
				}
				delete[] counter[i][j];
			}
			delete[] counter[i];
 		}
		delete[] counter;
		counter = nullptr;
		
		for (int i = 0; i < featureCount - 1; ++i)
		{
			delete[] condMutualInfo[i];
		}
		delete[] condMutualInfo;
		condMutualInfo = nullptr;

		// Print the parenting relations
		for (int i = 0; i < featureCount; i++)
		{
			NominalAttr &currentAttr = *(NominalAttr*)dataSet->_attrList[i];
			if (0 == i)
				fout << currentAttr._attrName << "  " << targetAttr._attrName << endl;
			else
			{
				NominalAttr &parentAttr = *(NominalAttr*)dataSet->_attrList[findDad[i]];
				fout << currentAttr._attrName << "  " << parentAttr._attrName << "  " << targetAttr._attrName << endl;
			}
		}
		fout << endl;

		// Test with the test set
		int correctCount = 0;
		NominalAttr &testTargetAttr = *(NominalAttr*)testDataSet->_attrList[attrCount - 1];
		double *prior = new double[targetValCount];
		for (int i = 0; i < targetValCount; i++)
		 	prior[i] = ((double) (targetCounter[i] + 1)) / (double)(trainEntryCount + targetValCount);
		for (int i = 0; i < testEntryCount; i++)
		{
			double *numerator = new double[targetValCount];
			for (int j = 0 ; j < targetValCount; j++)
			{
				numerator[j] = prior[j];
			}
			for (int j = 0; j < featureCount; j++)
			{
				NominalAttr &currentFeat = *(NominalAttr*)testDataSet->_attrList[j];
				if ( j == 0 )
				{
					for (int k = 0; k < targetValCount; k++)
					{
						numerator[k] *= condProb[j][currentFeat.getEntryValLabel(i)][k];
					}
				}
				else 
				{
					NominalAttr &parentFeat = *(NominalAttr*)testDataSet->_attrList[findDad[j]];
					for (int  k = 0; k < targetValCount; k++)
					{
						numerator[k] *= likelihood[j][currentFeat.getEntryValLabel(i)]\
							[parentFeat.getEntryValLabel(i)][k];
					}
				}
			}
			double normalization = 0;
			int bestGuess = 0;
			double bestProb;
			for (int j = 0; j < targetValCount; ++j)
			{
				normalization += numerator[j];
				if (j == 0)
					bestProb = numerator[j];
				else
				{
					if (numerator[j] > bestProb)
					{
						bestProb = numerator[j];
						bestGuess = j;
					}
				}
			}
			delete[] numerator;
			numerator = nullptr;
			fout << testTargetAttr._possibleValList[bestGuess] << " " << testTargetAttr._entryList[i] 
				<< " " << bestProb / normalization << endl;
			if  (testTargetAttr._possibleValList[bestGuess]  == testTargetAttr._entryList[i])
				correctCount++;
		}
		fout << endl << correctCount << endl;
		

		delete[] prior;
		prior = nullptr;

		for (int i = 0; i < featureCount; i++)
		 	delete[] condProb[i];
		delete[] condProb;
		condProb = nullptr;

		delete[] findDad;
		findDad = nullptr;

		delete[] targetCounter;
		targetCounter = nullptr;	
    }
    else 
	// Naive Bayes
    {
		for (int i = 0; i < attrCount-1; i++)
		{
			fout << ((NominalAttr*)dataSet->_attrList[i])->_attrName << "  " 
				<< ((NominalAttr*)dataSet->_attrList[attrCount-1])->_attrName << endl;
		}
		fout << endl;

		// Use training data to calculate prior probabilities and likelihood
		int featureCount = attrCount-1;
		NominalAttr &targetAttr = (*(NominalAttr*)dataSet->_attrList[featureCount]);
        int targetClassCount = targetAttr.getPossibleValCount();

        double ***likelihood = new double **[targetClassCount];
		for (int i = 0 ;  i < targetClassCount; i++)
		{
			likelihood[i] = new double* [featureCount];
			for (int j = 0; j < featureCount; j++)
			{
				int jClassCount = (*(NominalAttr*)dataSet->_attrList[j]).getPossibleValCount();
				likelihood[i][j] = new double [jClassCount];
				for (int k = 0; k < jClassCount; k++)
					likelihood[i][j][k] = 0.;
			}
		}
		double* prior = new double [targetClassCount];
		for (int i = 0; i < targetClassCount; i++)
		{
			prior[i] = 0.;
		}

		for (int i = 0; i < trainEntryCount; i++)
		{
			int targetLabel = targetAttr.getEntryValLabel(i);
			for (int j = 0; j < featureCount; j++)
			{
				likelihood[targetLabel][j][((NominalAttr*)dataSet->_attrList[j])->getEntryValLabel(i)] += 1.;
					// ->getValLabel(((NominalAttr*)dataSet->_attrList[j])->_entryList[i])] += 1.;
			}
			prior[targetLabel] += 1.;
		}

		for (int i = 0; i < targetClassCount; i++)
		{
			for (int j = 0; j < featureCount; j++)
			{
				for (int k = 0; k < ((NominalAttr*)dataSet->_attrList[j])->getPossibleValCount(); k++ )
				{
					likelihood[i][j][k] = (likelihood[i][j][k] + 1) 
						/ (prior[i] + ((NominalAttr*)dataSet->_attrList[j])->getPossibleValCount());
				}
			}
			prior[i] = (prior[i] + 1) / (trainEntryCount + targetAttr.getPossibleValCount());
		}
		
		// Use test set data to test the learned model
		NominalAttr &testTargetAttr = *(NominalAttr*)testDataSet->_attrList[featureCount];
		targetClassCount = testTargetAttr.getPossibleValCount();
		
		int correctCount = 0;
		for (int i = 0; i < testEntryCount; i++)
		{
			int bestGuess;
			double bestGuessProd;
			double *currentProd = new double [targetClassCount];
			double denominator = 0.;
			for (int j = 0; j < targetClassCount; j++)
			{
				currentProd[j] = 1.;
				for (int k = 0; k < featureCount; k++)
				{
					currentProd[j] *= likelihood[j][k]
						[((NominalAttr*)testDataSet->_attrList[k])->getEntryValLabel(i)];
				}
				currentProd[j] *= prior[j];
				denominator+= currentProd[j];

				if (0 == j)
				{
					bestGuessProd = currentProd[j];
					bestGuess = 0;
				}
				else
				{
					if (currentProd[j] > bestGuessProd)
					{
						bestGuess = j;
						bestGuessProd = currentProd[j];
					}
				}
			}
			fout << std::fixed << testTargetAttr._possibleValList[bestGuess] << "  " 
				<< testTargetAttr._entryList[i] << "  " << bestGuessProd / denominator << endl;  
			if (testTargetAttr._possibleValList[bestGuess] == testTargetAttr._entryList[i])
				correctCount++;
			delete[] currentProd;
			currentProd = nullptr;
		}
		fout << endl << correctCount << endl;

		// Garbage collection
		delete[] prior; 
		prior = nullptr;
		for (int i = 0; i < targetClassCount; i++)
		{
			for (int j = 0; j < featureCount; j++)
			{
				delete[] likelihood[i][j];
			}
			delete[] likelihood[i];
		}
		delete[] likelihood;
		likelihood = nullptr;
    }
	fout.close();
	
	delete testDataSet;
	delete dataSet;
    
    // cin.get();
    return 0;  
}
