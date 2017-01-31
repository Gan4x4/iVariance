#include <stdexcept>
#include <algorithm>    // std::max, replace
#include <cstdlib>
#include <iostream>
#include <iomanip>      // std::setw
#include <fstream>
#include <sstream>
#include <map>
#include <vector>
#include <cmath>        //pow

using namespace std;
typedef map<int,vector<double>> Container;

Container dat2map(const char* filename){
    ifstream file(filename,ios::in);
    if (file.good())
    {
        string str;
        Container result;
        int lastLen = 0;
        cout << "Read file: " << filename << "\n";
        int lNum = 0;
        int ignoredCount = 0;
        while(getline(file, str)) 
        {
            std::replace( str.begin(), str.end(), ',', '.'); // replace alll comma to dot
            istringstream ss(str);
            double num;
            vector<double> currentLine;
            while(ss >> num)
            {
                //result[i].push_back(num); 
                currentLine.push_back(num);
            }
            if (lastLen > 0 && lastLen != currentLine.size() ){
                std::cout << "Line " << lNum  << " has hole or zero in it :\n" << str << " \n";
                std::cout << "this line ignored \n\n";
                ignoredCount++;
            }
            else{
                for (size_t i=0; i< currentLine.size(); i++){
                    result[i].push_back(currentLine[i]);
                }
            }
            lastLen = currentLine.size();
            if (lNum % 1000 == 0 ) cout << ".";
            lNum ++;
        }
        cout << "\nRead " << result[0].size() << " lines\n";
        if (ignoredCount > 0){
            cout << "Ignored  " << ignoredCount << " lines\n";
        }
        return result;
    }  
    throw invalid_argument("Error while read file"); 
}

int getSpaces(const vector<double> &column){
    double maxElInCol = *(max_element(column.begin(),column.end()));
    int w = 9; // default width
    while (maxElInCol > 1000){
        maxElInCol =  maxElInCol/ 10;
        w++;
    }    
    return w;
}


void map2dat(const Container &data,string filename){
    ofstream myfile(filename);
    int maxColumnLen = 0;
    double maxNum = 0;
    vector<int> widths(data.size()); // num of space before number
    for (auto column : data){
        int tmp = column.second.size();
        maxColumnLen = max(tmp,maxColumnLen);
        widths[column.first] =  getSpaces(column.second); 
    }
    
    for (int i = 0;i< maxColumnLen;i++){
        myfile << std::setw(5) << i+1 ;
        for (auto column : data)
        {   
            if (column.second.size() >= i){
                myfile << std::setw(widths[column.first]) << fixed << setprecision(3) << column.second[i] ;
            }
            else{
                myfile << " ";
            }
        }
        myfile << "\n";
    }
    myfile.close();
    
}
        

double getMean(vector<double> data)
{
    double summ = 0;
    for(auto value : data){
        summ += value;
    }
    return summ/data.size();
}

double variance(vector<double> data)
{
   double mean = getMean(data);
   double summ= 0;
   for (auto x: data){
       summ +=  pow(x - mean,2);
   }
   return summ/(data.size()-1);
}


vector<double> convolution(vector<double> data,int by, int iterations){
    if (by == 1) {
        return data;
    }
    vector<double> result;
    if (by < 1 || by> iterations){
        throw invalid_argument("Invalid step " + by);
    }
    if (data.size() < by){
        return result;
    }
    int i = 0;
    double summ = 0;
    for (auto value : data){
        summ += value;
        i++;
        if (i == by){
           result.push_back(summ);
           // Tail is ignored
           summ = 0;
           i = 0;
        }
    }
    return result;
}

string getNewFileName(string oldFilename)
{
    string newFilename;
    string ext;
    string add = "_var";
    int dotPos = oldFilename.find(".");
    
    if (dotPos != string::npos){
        newFilename = oldFilename.substr(0,dotPos);
        ext = oldFilename.substr(dotPos,oldFilename.length()-1); 
        newFilename += add + ext;
    }
    else{
        newFilename = oldFilename + add;
    }
    
    return newFilename;
}

int main(int argc, char** argv)
{
    if (argc < 2) 
    {
        std::cout << "Usage: " << argv[0] << " data file name\n\n";
        return EXIT_FAILURE;
    }
    
    int iterations = 100;
    if (argc == 3){
        iterations = atoi(argv[2]);
    }
    
    if (iterations < 2){
        std::cout << "Iteration count invalid:  \n\n";
    }
    
    try
    {
        // Read file
        Container data = dat2map(argv[1]);
        Container varianceTable;
        cout << "\n";
        int i = 0;
        for(auto col : data)
        {
            for(int by = 1; by <= iterations; by++)
            {
                vector<double> conv = convolution(col.second, by,iterations);
                if (conv.size() == col.second.size() && by > 1)
                {
                    // данные закончились
                    break;
                }
                double var = variance(conv);
                varianceTable[col.first].push_back(var);
            }
            cout << "Column " << i << " has " << col.second.size() << " items  and calculated at " <<  varianceTable[col.first].size() << " Iterations \n";
            i++;
        }
        // write data to file
        string newFileName = getNewFileName(argv[1]);
        map2dat(varianceTable,newFileName);
        cout << "Data saved to " << newFileName << "\n";
    }
    catch (exception& e)
    {
        std::cout << e.what();
        return EXIT_FAILURE;
    
    }
    return EXIT_SUCCESS;
}

