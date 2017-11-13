#include <iostream>
#include <fstream>
#include <sstream>
#include <string>
#include <cmath>
#include <cfloat>
#include <cstdlib>
#include "DataSet.hpp"
#include "DataPrep.hpp"

using namespace std;

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
	system("rm -rf temp.sh attr.tmp attr2.tmp data.tmp data2.tmp attrTest attrTrain");

	ofstream fout;
	fout.precision(8);

	fout.open("result.dat", ios::out);
    if (isTAN)
    {
		// TODO: finish TAN NB Algo
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

		// Calculate conditional probability with Laplacian sum
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
		
		delete[] targetCounter;
		targetCounter = nullptr;	

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


		// double* condProb = new double [featureCount];
		// double** mutualCondProb = new double *[featureCount-1];
		// for (int i = 0; i < featureCount; i++)
		// {
		// 	if (i != featureCount-1)
		// 	{
		// 		mutualCondProb[i] = new double [featureCount-i];
		// 		condProb[i] = 0;
		// 		for (int j = 0; j < featureCount-i; j++)
		// 		{
		// 			int otherFeatIndex = i + j + 1;
		// 			mutualCondProb[i][j] = 0;
		// 		}
		// 	}
		// 	else
		// 	{
		// 		// TODO: deal with tail case: calculate condProb
		// 	}
		// }





		for (int i = 0; i < attrCount; i++)
		{
			for(int j = 0; j < attrCount; j++)
				delete[] mutualCondProb[i][j];
			delete[] mutualCondProb[i];
		}
		delete[] mutualCondProb;
		mutualCondProb = nullptr;
    }
    else 
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
		fout << correctCount << endl;

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
