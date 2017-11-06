#ifndef DATASET_HPP
#define DATASET_HPP

#include <iostream>
#include <fstream>
#include <cmath>
#include <string>


using namespace std;

class DTree;
class Node;
class NominalAttr;
class NumericalAttr;
class AbsAttr;

int compare(const void * a, const void * b)
{
	if (*(double*)a > *(double*)b) return 1;
	else if (*(double*)a < *(double*)b) return -1;
	else return 0;
}

static const string emptyStr = "";

class AbsAttr
{
	friend class DataSet;
	friend class NominalAttr;
	friend class NumericalAttr;
	friend class DTree;
	friend class Node;
public:
	AbsAttr() {};
	virtual ~AbsAttr() {};
	virtual double entropy() const = 0;
	virtual double condEntropy(NominalAttr const &attr2) = 0;
	//	virtual void append(const string &rhs) = 0;
	//	virtual void append(const double rhs) = 0;
	virtual int getEntryCount() const = 0;
	virtual bool isNominal() const = 0;
};

class NumericalAttr : public AbsAttr
{
	friend class AbsAttr;
	friend class NominalAttr;
	friend class DataSet;
	friend class DTree;
	friend class Node;

// private:
public:
	string _attrName;
	int _entryCount;
	double *_entryList;
	double _splitPoint;
public:
	NumericalAttr(string const &attrName, int entryCount, istream &entrySource);
	NumericalAttr(int entryCount, NumericalAttr const &example);
	NumericalAttr(NumericalAttr const &example);
	~NumericalAttr();
	double entropy() const override; // disabled
	double condEntropy(NominalAttr const &attr2) override;
	//	void append(const double entry) override;
	int getEntryCount() const override;
	bool isNominal() const override;
};

class NominalAttr : public AbsAttr
{
	friend class AbsAttr;
	friend class NumericalAttr;
	friend class DataSet;
	friend class DTree;
	friend class Node;
// 
public:
	string _attrName;
	int _entryCount;
	int _possibleValCount;
	string *_possibleValList;
	string *_entryList;

public:
	NominalAttr(string const &attrName, int possibleValCount, istream &possibleValSource, int entryCount, istream &entrySource);
	NominalAttr(int entryCount, NominalAttr const &example);
	NominalAttr(NominalAttr const &example);
	~NominalAttr();
	double entropy() const override;
	double condEntropy(NominalAttr const &attr2) override;
	//	void append(const string &entry) override;
	int getEntryCount() const override;
	int getPossibleValCount() const;
	bool isThat(string const &entry, int possibleValLabel) const;
	bool isNominal() const override;
	// check if entry matches the possibleValLabel-st in the possibleVals list
	// pre: possibleValLabel must not be larger than possibleValCount
	int getValLabel(string const &entry) const;

	int getEntryValLabel(int index) const;
	// Pre: index < _entryCount
};


class DataSet
{
	friend class AbsAttr;
	friend class NominalAttr;
	friend class NumericalAttr;
	friend class DTree;
	friend class Node;
// private:
public:
	int _attrCount;
	AbsAttr **_attrList;
public:
	DataSet();
	~DataSet();
	DataSet(int attrCount, AbsAttr **attrList);
	void split(DataSet *&children, AbsAttr *&splitAttr);
	int getAttrCount() const;
	void setDataSet(int attrCount, AbsAttr **attrList);
	int getEntryCount() const;
	bool ifStop(int stopThreshold, string &label) const;
	string const &getMajorityLabel() const;
	// Pre: the dataset is not empty
};

//===============constructors=================
DataSet::DataSet() :
	_attrList(nullptr),
	_attrCount(0)
{}

DataSet::DataSet(int attrCount, AbsAttr **attrs) :
	_attrCount(attrCount),
	_attrList(attrs)
{}


NumericalAttr::NumericalAttr(string const &attrName, int entryCount, istream &entrySource) :
	_entryCount(entryCount),
	_attrName(attrName),
	_entryList(new double[entryCount])
{
	for (int i = 0; i < entryCount; i++)
	{
		entrySource >> _entryList[i];
	}

	_splitPoint = _entryList[0];
}

NumericalAttr::NumericalAttr(int entryCount, NumericalAttr const &example) :
	_entryCount(entryCount),
	_entryList(new double[entryCount])
{
	_attrName = example._attrName;
}
NumericalAttr::NumericalAttr(NumericalAttr const &example) :
	_entryCount(0),
	_entryList(nullptr)
{
	_splitPoint = example._splitPoint;
	_attrName = example._attrName;
}

NominalAttr::NominalAttr(string const &attrName, int possibleValCount, istream &possibleValSource, int entryCount, istream &entrySource) :
	_attrName(attrName),
	_possibleValCount(possibleValCount),
	_entryCount(entryCount),
	_possibleValList(new string[possibleValCount]),
	_entryList(new string[entryCount])
{
	for (int i = 0; i < _possibleValCount; i++)
	{
		possibleValSource >> _possibleValList[i];
	}
	for (int j = 0; j < _entryCount; j++)
	{
		entrySource >> _entryList[j];
	}
}

NominalAttr::NominalAttr(int entryCount, NominalAttr const &example) :
	_entryCount(entryCount),
	_entryList(new string[entryCount])
{
	_attrName = example._attrName;
	_possibleValCount = example._possibleValCount;
	_possibleValList = new string[_possibleValCount];
	for (int i = 0; i < _possibleValCount; i++)
		_possibleValList[i] = example._possibleValList[i];
}
NominalAttr::NominalAttr(NominalAttr const &example) :
	_entryCount(0),
	_entryList(nullptr)
{
	_attrName = example._attrName;
	_possibleValCount = example._possibleValCount;
	_possibleValList = new string[_possibleValCount];
	for (int i = 0; i < _possibleValCount; i++)
		_possibleValList[i] = example._possibleValList[i];
}

//====================Destructor===================
NumericalAttr::~NumericalAttr()
{
	delete[] _entryList;
	_entryList = nullptr;
}
NominalAttr::~NominalAttr()
{
	delete[] _entryList;
	_entryList = nullptr;
	delete[] _possibleValList;
	_possibleValList = nullptr;
}
DataSet::~DataSet()
{
	for (int i = 0; i < _attrCount; i++)
	{
		if (_attrList[i]->isNominal())
			delete (NominalAttr*)_attrList[i];
		else
			delete (NumericalAttr*)_attrList[i];
	}
	delete[] _attrList;
	_attrList = nullptr;
}

//==================getEntryCount========================

int DataSet::getEntryCount() const
{
	if (0 == _attrCount)
		return 0;
	else
		return (*_attrList[0]).getEntryCount();
}

int NominalAttr::getEntryCount() const
{
	return _entryCount;
}
int NumericalAttr::getEntryCount() const
{
	return _entryCount;
}
//========================getPossibleValCount===================
int NominalAttr::getPossibleValCount() const
{
	return _possibleValCount;
}

//================isThat================
bool NominalAttr::isThat(string const &entry, int possibleValLabel) const
{
	if (possibleValLabel > _possibleValCount - 1)
	{
		cerr << "Over the range of possible values!!!\n";
		throw(-1);
	}
	return _possibleValList[possibleValLabel] == entry;
}

//=======================ifStop===========================
bool DataSet::ifStop(int stopThreshold, string &label) const
// label should carry the majority label of parent dataSet
{
	int entryCount = getEntryCount();
	if (0 == entryCount)
		return true;
	NominalAttr &targetAttr = (*(NominalAttr*)_attrList[_attrCount - 1]);
	if (entryCount < stopThreshold)
	{
		if (0 != entryCount)
		{
			int *counter = new int[targetAttr.getPossibleValCount()];
			for (int j = 0; j < targetAttr.getPossibleValCount(); j++)
				counter[j] = 0;
			for (int k = 0; k < entryCount; k++)
				counter[targetAttr.getValLabel(targetAttr._entryList[k])]++;
			int bestLabel = 0;
			for (int n = 1; n < targetAttr.getPossibleValCount(); n++)
			{
				if (counter[n] == counter[bestLabel])
					return true;
				if (counter[n] > counter[bestLabel])
					bestLabel = n;
			}
			label = targetAttr._possibleValList[bestLabel];
		}
		return true;
	}

	for (int i = 0; i < entryCount - 1; i++)
		// check if all of the entries in this dataset have the same label
	{
		if (targetAttr._entryList[i] !=
			targetAttr._entryList[entryCount - 1])
			return false;
	}
	label = targetAttr._entryList[0];
	return true;
}

//===================getValLabel==========================
int NominalAttr::getValLabel(string const &entry) const
{
	for (int i = 0; i < _possibleValCount; i++)
	{
		if (isThat(entry, i))
			return i;
	}
	cerr << entry << endl;
	cerr << "Entry not in the possibleValList!!!\n";
	throw(-1);
}

//=======================setDataSet========================
void DataSet::setDataSet(int attrCount, AbsAttr **attrList)
{
	_attrCount = attrCount;
	_attrList = attrList;
}

//==========================entropy=============================
double NominalAttr::entropy() const
{
	int *valCount = new int[_possibleValCount];
	double result = 0;
	for (int i = 0; i < _possibleValCount; i++)
	{
		valCount[i] = 0;
	}
	for (int j = 0; j < _entryCount; j++)
	{
		valCount[getValLabel(_entryList[j])]++;
	}
	for (int k = 0; k < _possibleValCount; k++)
	{
		result -= ((double)valCount[k]) / ((double)_entryCount)*log2(((double)valCount[k]) / ((double)_entryCount));
	}
	delete[] valCount;
	valCount = nullptr;
	return result;
}
double NumericalAttr::entropy() const
{
	cerr << "Function Disabled!!!\n";
	throw (-1);
}

//=====================condEntropy======================
double NominalAttr::condEntropy(NominalAttr const &attr2)
{
	int **occurence = new int*[attr2._possibleValCount];
	for (int x = 0; x < attr2._possibleValCount; x++)
	{
		occurence[x] = new int[_possibleValCount];
		for (int y = 0; y < _possibleValCount; y++)
			occurence[x][y] = 0;
	}

	for (int i = 0; i < _entryCount; i++)
	{
		occurence[attr2.getValLabel(attr2._entryList[i])][getValLabel(_entryList[i])]++;
	}

	double result = 0;
	for (int j = 0; j < _possibleValCount; j++)
	{
		int	attr1Counter = 0;
		for (int k = 0; k < attr2._possibleValCount; k++)
		{
			attr1Counter += occurence[k][j];
		}

		double probAttr1 = ((double)attr1Counter) / ((double)_entryCount), plgp = 0;
		for (int l = 0; l < attr2._possibleValCount; l++)
		{
			double probAttr2GivenAttr1 = attr1Counter == 0 ? 0 : ((double)occurence[l][j]) / ((double)attr1Counter);
			plgp += probAttr2GivenAttr1*log2(probAttr2GivenAttr1);
		}
		result -= probAttr1*plgp;
	}

	for (int x = 0; x < attr2._possibleValCount; x++)
	{
		delete[] occurence[x];
		occurence[x] = nullptr;
	}
	delete[] occurence;
	occurence = nullptr;
	return result;
}

double NumericalAttr::condEntropy(NominalAttr const &attr2)
{
	double *entryClone = new double[_entryCount];
	for (int m = 0; m < _entryCount; m++)
	{
		entryClone[m] = _entryList[m];
	}
	qsort(entryClone, _entryCount, sizeof(double), compare);

	double bestCondEntropy, bestSplitPoint;
	for (int n = 0; n < _entryCount - 1; n++)//number of possible splitpoints
	{
		double splitPoint = (entryClone[n] + entryClone[n + 1]) / 2.0;

		if (0 == n)
			bestSplitPoint = splitPoint;

		int **occurence = new int*[2];
		for (int x = 0; x < 2; x++)
		{
			occurence[x] = new int[attr2._possibleValCount];
			for (int y = 0; y < attr2._possibleValCount; y++)
				occurence[x][y] = 0;
		}

		for (int i = 0; i < _entryCount; i++)
		{
			occurence[(_entryList[i] <= splitPoint ? 0 : 1)][attr2.getValLabel(attr2._entryList[i])]++;
		}

		double result = 0;
		for (int j = 0; j < 2; j++)
		{
			int	attr1Counter = 0;
			for (int k = 0; k < attr2._possibleValCount; k++)
			{
				attr1Counter += occurence[j][k];
			}

			double probAttr1 = 0 == _entryCount ? 0 : (((double)attr1Counter) / ((double)_entryCount));
			double plgp = 0;
			for (int l = 0; l < attr2._possibleValCount; l++)
			{
				double probAttr2GivenAttr1 = attr1Counter == 0 ? 0 : ((double)occurence[j][l]) / ((double)attr1Counter);
				plgp += probAttr2GivenAttr1<(1e-8) ? 0 : probAttr2GivenAttr1*log2(probAttr2GivenAttr1);
			}
			result -= probAttr1*plgp;
		}

		for (int x = 0; x < attr2._possibleValCount; x++)
		{
			delete[] occurence[x];
			//occurence[x] = nullptr;
		}
		delete[] occurence;
		occurence = nullptr;
		if (0 == n)
			bestCondEntropy = result;
		else if (result < bestCondEntropy - 1e-8)
		{
			bestCondEntropy = result;
			bestSplitPoint = splitPoint;
		}
	}

	delete[] entryClone;
	entryClone = nullptr;
	_splitPoint = bestSplitPoint;

	return bestCondEntropy;
}

//============================isNominal======================
bool NominalAttr::isNominal() const
{
	return true;
}
bool NumericalAttr::isNominal() const
{
	return false;
}

//===================getAttrCount=========================
int DataSet::getAttrCount() const
{
	return _attrCount;
}

//===================split=======================
void DataSet::split(DataSet *&children, AbsAttr *&splitAttr)
// returns the number of children created
{
	double bestInfoGain;
	int bestAttr = 0;
	double labelEntropy = (*_attrList[_attrCount - 1]).entropy();
	NominalAttr &targetAttr = *(NominalAttr*)_attrList[_attrCount - 1];
	for (int i = 0; i < _attrCount - 1; i++)
	{
		double infoGain = -(*_attrList[i]).condEntropy(targetAttr);
		if (0 == i)
			bestInfoGain = infoGain;
		else if (infoGain > bestInfoGain)
		{
			bestAttr = i;
			bestInfoGain = infoGain;
		}
	}

	if (!(*_attrList[bestAttr]).isNominal())
		//thesplit attribute is numerical
	{
		int entryNumber = (*_attrList[0]).getEntryCount();
		NumericalAttr &bestAttrRef = *(NumericalAttr*)_attrList[bestAttr];
		int counter[2];
		counter[0] = 0;
		counter[1] = 0;
		double splitPoint = bestAttrRef._splitPoint;
		for (int i = 0; i < (*_attrList[0]).getEntryCount(); i++)
		{
			if (bestAttrRef._entryList[i] <= splitPoint)
				counter[0]++;
			else
				counter[1]++;
		}
		children = new DataSet[2];
		AbsAttr ***newAttrList = new AbsAttr **[2];
		newAttrList[0] = new AbsAttr *[_attrCount];
		newAttrList[1] = new AbsAttr *[_attrCount];
		
		for (int j = 0; j < _attrCount; j++)
		{
			newAttrList[0][j] = (*_attrList[j]).isNominal() ?
				((AbsAttr*) new NominalAttr(counter[0], *(NominalAttr*)_attrList[j])) :
				((AbsAttr*) new NumericalAttr(counter[0], *(NumericalAttr*)_attrList[j]));
			newAttrList[1][j] = (*_attrList[j]).isNominal() ?
				((AbsAttr*) new NominalAttr(counter[1], *(NominalAttr*)_attrList[j])) :
				((AbsAttr*) new NumericalAttr(counter[1], *(NumericalAttr*)_attrList[j]));
		}
		int index1 = 0, index2 = 0;
		for (int m = 0; m < entryNumber; m++)
			//prepare the attribute for children
		{
			if (bestAttrRef._entryList[m] <= splitPoint)
			{
				for (int n = 0; n < _attrCount; n++)
				{
					if ((*_attrList[n]).isNominal())
						(*(NominalAttr*)newAttrList[0][n])._entryList[index1] = (*(NominalAttr*)_attrList[n])._entryList[m];
					else
						(*(NumericalAttr*)newAttrList[0][n])._entryList[index1] = (*(NumericalAttr*)_attrList[n])._entryList[m];
				}
				index1++;
			}
			else
			{
				for (int n = 0; n < _attrCount; n++)
				{
					if ((*_attrList[n]).isNominal())
						(*(NominalAttr*)newAttrList[1][n])._entryList[index2] = (*(NominalAttr*)_attrList[n])._entryList[m];
					else
						(*(NumericalAttr*)newAttrList[1][n])._entryList[index2] = (*(NumericalAttr*)_attrList[n])._entryList[m];
				}
				index2++;
			}
		}

		children[0].setDataSet(_attrCount, newAttrList[0]);
		children[1].setDataSet(_attrCount, newAttrList[1]);
		splitAttr = new NumericalAttr(*(NumericalAttr*)_attrList[bestAttr]);
		delete[] newAttrList;
		newAttrList = nullptr;
	}
	else
		// the split attribute is nominal
	{
		int splitNumber = (*(NominalAttr*)_attrList[bestAttr])._possibleValCount;
		int entryNumber = (*_attrList[0]).getEntryCount();
		NominalAttr &bestAttrRef = *(NominalAttr*)_attrList[bestAttr];
		int *counter = new int[splitNumber];
		for (int a = 0; a < splitNumber; a++)
			counter[a] = 0;

		for (int i = 0; i < entryNumber; i++)
		{
			counter[bestAttrRef.getValLabel(bestAttrRef._entryList[i])]++;
		}
		children = new DataSet[splitNumber];
		AbsAttr ***newAttrList = new AbsAttr **[splitNumber];
		for (int i = 0; i < splitNumber; i++)
			newAttrList[i] = new AbsAttr *[_attrCount];
		for (int j = 0; j < _attrCount; j++)
		{
			for (int k = 0; k < splitNumber; k++)
				newAttrList[k][j] = (*_attrList[j]).isNominal() ?
				((AbsAttr*) new NominalAttr(counter[k], *(NominalAttr*)_attrList[j])) :
				((AbsAttr*) new NumericalAttr(counter[k], *(NumericalAttr*)_attrList[j]));

		}
		int *index = new int[splitNumber];
		for (int k = 0; k < splitNumber; k++)
			index[k] = 0;
		for (int m = 0; m < entryNumber; m++)
			//prepare the attributes for children
		{
			int labelIndex = bestAttrRef.getValLabel(bestAttrRef._entryList[m]);
			for (int n = 0; n < _attrCount; n++)
			{
				if ((*_attrList[n]).isNominal())
					(*(NominalAttr*)newAttrList[labelIndex][n])._entryList[index[labelIndex]] = (*(NominalAttr*)_attrList[n])._entryList[m];
				else
					(*(NumericalAttr*)newAttrList[labelIndex][n])._entryList[index[labelIndex]] = (*(NumericalAttr*)_attrList[n])._entryList[m];
			}
			index[labelIndex]++;
		}

		for (int x = 0; x < splitNumber; x++)
			children[x].setDataSet(_attrCount, newAttrList[x]);
		splitAttr = new NominalAttr(*(NominalAttr*)_attrList[bestAttr]);
		delete[] index;
		index = nullptr;
		//delete[] newAttrList;
		newAttrList = nullptr;
	}

}

//=================getMajorityLabel==========================
string const &DataSet::getMajorityLabel() const
{
	if (_attrCount == 0)
	{
		cerr << "Dataset is empty!!!\n";
		throw (-1);
	}
	NominalAttr &targetAttr = *(NominalAttr*)_attrList[_attrCount - 1];
	int *counter = new int[targetAttr.getPossibleValCount()];
	for (int i = 0; i < targetAttr.getPossibleValCount(); i++)
		counter[i] = 0;
	for (int i = 0; i < getEntryCount(); i++)
	{
		counter[targetAttr.getValLabel(targetAttr._entryList[i])]++;
	}
	int majorLabelIndex = 0;
	for (int j = 1; j < targetAttr.getPossibleValCount(); j++)
	{
		if (counter[j] == counter[majorLabelIndex])
			return emptyStr;
		else if (counter[j] > counter[majorLabelIndex])
			majorLabelIndex = j;
	}
	delete[] counter;
	counter = nullptr;
	return targetAttr._possibleValList[majorLabelIndex];
}

//=================getEntryValLabel====================
int NominalAttr::getEntryValLabel(int index) const
{
	if (index >= _entryCount)
	{
		cerr << "Index given is larger than the capacity of entryList!!!\n";
		throw -1;
	}
	return getValLabel(_entryList[index]);
}

#endif