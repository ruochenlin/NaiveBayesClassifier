#ifndef DATAPREP_HPP_
#define DATAPREP_HPP_
#include<iostream>
#include<fstream>
#include <string>
#include <cstdlib>
using namespace std;
void dataPrep(string const &trainFile, string const &testFile)
{
	ofstream fout;
	fout.open("temp.sh");
	fout << "#!/bin/bash" << endl <<
		"rm -rf attrTrain" << endl <<
		"lineCount=`wc -l " << trainFile << "  | awk '{print $1 }'`" << endl <<
		"attrCount=`grep -c attribute " << trainFile << " `" << endl <<
		"entryCount=$(($((lineCount)) - 2 - $((attrCount)))) " << endl <<
		"echo \"$attrCount $entryCount\"> attr.tmp " << endl <<
		"grep attribute " << trainFile << "  | sed -e 's/{/ nominal /g' | sed -e 's/numeric/real/g' | sed -e 's/}//g' | sed -e \"s/' / /g\" | awk -F=\"attribute\" '{print }' | awk -F \"'\" '{print $2}' | sed -e 's/,/ /g' | sed -e 's/  */ /g' >> attr.tmp " << endl <<
		"tail -$((entryCount)) " << trainFile << "  | sed -e 's/,/ /g' > data.tmp " << endl <<
		"mkdir attrTrain " << endl <<
		"for ((i=1; i <= $attrCount; i++)) " << endl <<
		"do " << endl <<
		"cat data.tmp | awk '{print $var}' var=$i > attrTrain/$i  " << endl <<
		"done " << endl << endl <<
		"rm -rf attrTest " << endl <<
		"lineCount2=`wc -l " << testFile << " | awk '{print $1 }'` " << endl <<
		"entryCount2=$(($((lineCount2)) - 2 - $((attrCount)))) " << endl <<
		"echo \"$attrCount $entryCount2\" > attr2.tmp " << endl <<
		"grep attribute " << testFile << " | sed -e 's/{/ nominal /g' | sed -e 's/numeric/real/g' | sed -e 's/}//g' | sed -e \"s/' / /g\" | awk -F \"'\" '{print $2}' | sed -e 's/,/ /g' | sed -e 's/  */ /g' >> attr2.tmp " << endl <<
		"tail -$((entryCount2)) " << testFile << " | sed -e 's/,/ /g' > data2.tmp " << endl <<
		"mkdir attrTest " << endl <<
		"for ((j=1; j <= $attrCount; j++)) " << endl <<
		"do " << endl <<
		"cat data2.tmp | awk '{print $var}' var=$j > attrTest/$j " << endl <<
		"done";
	fout.close();
	system("chmod +x temp.sh");
	system("./temp.sh");
}
#endif
