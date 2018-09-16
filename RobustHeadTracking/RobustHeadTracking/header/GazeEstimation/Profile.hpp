//
//  Profile.hpp
//  RobustHeadTracking
//
//  Created by Kevin Bai on 2017-04-27.
//  Copyright © 2017 Kevin Bai. All rights reserved.
//

#ifndef Profile_hpp
#define Profile_hpp

#include <iostream>
#include <vector>
#include <cmath>
#include <fstream>

using namespace std;

class Range;

class Profile {
public:
    int headYaw;
    int headHeight;
    
    vector<Range> gridRanges;
    
    Profile();
    Profile(int yaw, int height);
    Profile(string filename);
    
    //return best grid, but also return possibilities
    int predict(int pupilCoordinate, vector<int> &possibilities);
    
private:
    void parseLine(string line, double *rX, double *rY, double *rZ, double *rectX, double *rectY, double *rectWidth, double *rectHeight, double *pupilX, double *pupilY, double *grid);
    vector<double> splitStr(string line);
};

class Range {
public:
    vector<int> values;
    int min = -1, max = -1;
    double average = 0;
    
    Range();
    Range(int min, int max);
    
    void append(int num);
    bool contains(int value);

};

#endif /* Profile_hpp */
