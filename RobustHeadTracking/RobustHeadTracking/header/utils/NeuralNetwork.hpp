//
//  NeuralNetwork.hpp
//  NeuralNetwork
//
//  Created by Kevin Bai on 2017-04-15.
//  Copyright © 2017 Kevin Bai. All rights reserved.
//

#ifndef NeuralNetwork_hpp
#define NeuralNetwork_hpp

#include <iostream>
#include <vector>
#include <math.h>
#include <Eigen/Sparse>
#include <opencv2/opencv.hpp>

using namespace std;
using namespace Eigen;

//vector printer
inline string toString(vector<int> v) {
    string s = "[";
    for (int i = 0; i < v.size(); i++) {
        s += to_string(v[i]);
        if (i != v.size() - 1) s += ", ";
    }
    return s + "]";
}

inline vector<string> splitStr(string s, char separator) {
    vector<string> v;
    v.push_back("");
    int vCounter = 0;
    for (int i = 0; i < s.size(); i++) {
        if (s[i] != separator) {
            v[vCounter] += s[i];
        }
        else {
            if (v[vCounter].size() > 0) {
                vCounter += 1;
                v.push_back("");
            }
        }
    }
    return v;
}

inline double relu(double x) {
    return (x <= 0 ? 0 : x);
}
inline double customExp(double x) {
    return exp(x);
}
inline double sq(double x) {
    return x*x;
}
inline double customRound(double x) {
    return round(x);
}

class MultiLayerNN {
public:
    //hyper parameters
    double reg = 0.001;
    double stepSize = 0.5;
    bool resavingData = false;

    
    MultiLayerNN();
    MultiLayerNN(string file);
    MultiLayerNN(vector<int> layerValues);
    
    //specify the number of layers and num of neurons in each layer
    vector<int> layers;
    //weights
    vector<MatrixXd> weights;
    //biases
    vector<RowVectorXd> bias;
    
    //make a prediction
    double predict(RowVectorXd input);
    
    //traing a set of data
    void batchTrain(int iterations, MatrixXd input, VectorXd label);
    double testAccuracy(MatrixXd input, VectorXd labels);
    
    void setFile(string filename, bool saveData);
    void save(string filename);
    void save();
private:
    string saveFile;
    
    double currentLoss;
    double currentAccuracy;
    
    //matrix functions
    MatrixXd divideRowwiseSum(MatrixXd m);
    MatrixXd computeLogProbs(MatrixXd m, VectorXd labels);
    MatrixXd computeGradientDescent(MatrixXd m, VectorXd labels);
};


#endif /* NeuralNetwork_hpp */
