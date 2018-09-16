//
//  Profile.cpp
//  RobustHeadTracking
//
//  Created by Kevin Bai on 2017-04-27.
//  Copyright © 2017 Kevin Bai. All rights reserved.
//

#include "Profile.hpp"

Profile::Profile() { }
Profile::Profile(int yaw, int height) {
    headYaw = yaw;
    headHeight = height;
}
Profile::Profile(string filename) {
    ifstream file(filename);
    
    //create ranges for 5 grids
    for (int i = 0; i < 5; i++) gridRanges.push_back(Range(1000, 0));
    
    for (int i = 0; i < 50; i++) {
        string line;
        getline(file, line);
        double rX, rY, rZ, rectX, rectY, rectWidth,rectHeight, pupilX, pupilY, grid;
        parseLine(line, &rX, &rY, &rZ, &rectX, &rectY, &rectWidth, &rectHeight, &pupilX, &pupilY, &grid);
        
        //adjust the range
        gridRanges[i / 10].append(pupilX);
        
        //set corresponding head height and rotation
        if (i == 0) {
            //match a corresponding profile for head rotation
            headYaw = round(rY / 10) * 10;
            if (headYaw > 20) headYaw = 20;
            if (headYaw < -20) headYaw = -20;
            
            //match correspoinding profile for head height
            headHeight = 250; //height = 250
            if (rectHeight <= 175) headHeight = 150;
            else if (rectHeight <= 225) headHeight = 200;
        }
    }
    file.close();
}

int Profile::predict(int x, vector<int> &possibleRanges) {
    for (int i = 0; i < 5; i++) {
        if (gridRanges[i].contains(x)) possibleRanges.push_back(i);
    }
    
    if (possibleRanges.size() == 0) {
        //if it did not fit into any range
        int leftOfGrid = -1;
        for (int i = 0; i < 5; i++) {
            if (x > gridRanges[i].max) {
                leftOfGrid = i;
            }
        }
        //determine the closest grid to pupil at the same time
        int closestGrid = leftOfGrid;
        if (leftOfGrid != -1 && leftOfGrid != 4) {
            if (abs(gridRanges[leftOfGrid].max - x) > abs(gridRanges[leftOfGrid+1].min - x)) closestGrid += 1;
            //range to left and right of x is a possibility
            possibleRanges.push_back(leftOfGrid);
            possibleRanges.push_back(leftOfGrid+1);
        }
        else if (leftOfGrid == -1) {
            possibleRanges.push_back(0);
            return 0;
        }
        else possibleRanges.push_back(4);
        
        //return the best grid
        return closestGrid;
    }
    else if (possibleRanges.size() == 1) {
        //if fit into 1 range
        return possibleRanges[0];
    }
    else {
        //if more than 2 regions were detected than find the closest average
        int closest = 1000;
        int closestIndex = 0;
        for (int i = 0; i < possibleRanges.size(); i++) {
            if (abs(gridRanges[possibleRanges[i]].average - x) < closest) {
                closest = gridRanges[possibleRanges[i]].average;
                closestIndex = possibleRanges[i];
            }
        }
        return closestIndex;
    }
}

vector<double> Profile::splitStr(string line) {
    vector<double> returnVec;
    string current = "";
    for (int i = 0; i < line.size(); i++) {
        if (line[i] != ' ') current += line[i];
        else {
            //parse string
            returnVec.push_back(stof(current));
            current = "";
        }
    }
    returnVec.push_back(stof(current));
    
    return returnVec;
}

void Profile::parseLine(string line, double *rX, double *rY, double *rZ, double *rectX, double *rectY, double *rectWidth, double *rectHeight, double *pupilX, double *pupilY, double *grid) {
    /*isolate to :
     *  rX rY rZ -> xyzStr
     *  rectX rectY rectWidth rectHeight) -> rectStr
     *  pupilX pupilY -> pupilStr
     *  grid 0 - gridStr
     */
    string xyzStr = "";
    string rectStr = "";
    string pupilStr = "";
    string gridStr = "";
    
    bool passedXYZ = false;
    bool passedRect = false;
    bool passedPupil = false;
    for (int i = 0; i < line.size(); i++) {
        if (!passedXYZ) {
            if (line[i] == ']') passedXYZ = true;
            else if (line[i] != '[' && line[i] != ',') xyzStr += line[i];
        }
        else if (!passedRect) {
            if (line[i] == ')') passedRect = true;
            else if (line[i] != '(' && line[i] != ',' && line[i] != ':') rectStr += line[i];
        }
        else if (!passedPupil) {
            if (line[i] == ')') passedPupil = true;
            else if (line[i] != '(' && line[i] != ',' && line[i] != ':') pupilStr += line[i];
        }
        else {
            if (line[i] == ')') passedPupil = true;
            else if (line[i] != '(' && line[i] != ',' && line[i] != ':' && line[i] != '-' && line[i] != '>') gridStr += line[i];
        }
    }
    
    //simply split each string by " "
    vector<double> xyz = splitStr(xyzStr);
    vector<double> rectdoubles = splitStr(rectStr);
    vector<double> pupildoubles = splitStr(pupilStr);
    
    *rX = xyz[0];
    *rY = xyz[1];
    *rZ = xyz[2];
    *rectX = rectdoubles[0];
    *rectY = rectdoubles[1];
    *rectWidth = rectdoubles[2];
    *rectHeight = rectdoubles[3];
    *pupilX = pupildoubles[0];
    *pupilY = pupildoubles[1];
    *grid = (int) gridStr[0] - 48;
}

Range::Range() { }
Range::Range(int min, int max) {
    this->min = min;
    this->max = max;
}

bool Range::contains(int value) {
    if (value >= min && value <= max)
        return true;
    return false;
}

void Range::append(int num) {
    //adjust the average
    average = (average * values.size() + num) / (values.size() + 1);
    values.push_back(num);
    
    //update min and max values
    if (min == -1) min = num;
    else if (num < min) min = num;
    if (max == -1) max = num;
    else if (num > max) max = num;
}
