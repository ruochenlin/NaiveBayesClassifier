#include <iostream>
#include <fstream>
#include <sstream>
#include <string>
#include <cmath>
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
    
    if (isTAN)
    {
        // TODO: finish TAN NB Algo
    }
    else 
    {
		int featureCount = attrCount-1;
		NominalAttr &targetAttr = (*(NominalAttr*)dataSet->_attrList[featureCount]);
        int targetClassCount = targetAttr.getEntryCount();

        double ***likelihood = new double **[targetClassCount];
		for (int i = 0 ;  i < targetClassCount; i++)
		{
			likelihood[i] = new double* [featureCount];
			for (int j = 0; j < featureCount; j++)
			{
				int jClassCount = (*(NominalAttr*)dataSet->_attrList[featureCount]).getPossibleValCount();
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
			int targetLabel = targetAttr.getValLabel(targetAttr._entryList[i]);
			for (int j = 0; j < featureCount; j++)
			{
				likelihood[targetLabel][j][((NominalAttr*)dataSet->_attrList[j])
					->getValLabel(((NominalAttr*)dataSet->_attrList[j])->_entryList[i])] += 1.;
			}
			prior[targetLabel] += 1.;
		}
		for (int i = 0; i < targetClassCount; i++)
		{
			for (int j = 0; j < featureCount; j++)
			{
				for (int k = 0; k < ((NominalAttr*)dataSet->_attrList[j])->getPossibleValCount(); k++ )
				{
					likelihood[i][j][k] /= prior[i];
				}
			}
			prior[i] /= trainEntryCount;
		}

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
    



    delete testDataSet;
    



    system("rm -rf temp.sh attr.tmp attr2.tmp data.tmp data2.tmp attrTest attrTrain");
    
    cin.get();
    return 0;  
}
