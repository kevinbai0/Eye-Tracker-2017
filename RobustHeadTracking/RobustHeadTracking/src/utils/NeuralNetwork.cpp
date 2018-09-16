//
//  NeuralNetwork.cpp
//  NeuralNetwork
//
//  Created by Kevin Bai on 2017-04-15.
//  Copyright © 2017 Kevin Bai. All rights reserved.
//

#include "../../header/utils/NeuralNetwork.hpp"

MultiLayerNN::MultiLayerNN() {}
MultiLayerNN::MultiLayerNN(string filename) {
    ifstream file(filename);
    
    string line;
    getline(file, line);
    //parse title
    vector<string> elems = splitStr(line, ' ');
    
    //get number of layers
    string numLayersStr = "";
    for (int i = 1; i < elems[3].size() - 1; i++) numLayersStr += elems[3][i];
    int numLayers = stoi(numLayersStr);
    
    for (int i = (int) elems.size() - numLayers; i < elems.size(); i++) {
        layers.push_back(stoi(elems[i]));
    }
    
    //create empty weights and biases
    for (int i = 0; i < numLayers - 1; i++) weights.push_back(MatrixXd(layers[i], layers[i+1]));
    for (int i = 1; i < numLayers; i++) bias.push_back(RowVectorXd(layers[i]));
    
    //discard the next 2 lines
    getline(file, line);
    getline(file, line);
    
    for (int i = 0; i < numLayers-1; i++) {
        for (int j = 0; j < weights[i].rows(); j++) {
            getline(file, line);
            vector<string> colValues = splitStr(line, ' ');
            for (int k = 0; k < colValues.size(); k++) weights[i](j,k) = stod(colValues[k]);
        }
        //discard one line
        getline(file, line);
    }
    
    //discard next line:
    getline(file, line);
    
    for (int i = 0; i < numLayers - 1; i++) {
        getline(file, line);
        vector<string> vals = splitStr(line, ' ');
        for (int j = 0; j < bias[i].size(); j++) {
            bias[i](j) = stod(vals[j]);
        }
        //discard one line
        getline(file, line);
    }
    
    file.close();
    
    saveFile = filename;
}
MultiLayerNN::MultiLayerNN(vector<int> layerValues) {
    layers = layerValues;
    
    for (int i = 0; i < layerValues.size(); i++) {
        if (i != layerValues.size() - 1) {
            srand(1);
            weights.push_back(0.01 * MatrixXd::Random(layerValues[i], layerValues[i+1]));
        }
    }
    for (int i = 0; i < layerValues.size(); i++) {
        if (i != 0) {
            //  bias.push_back(RowVectorXd::Random(layerValues[i]));
            bias.push_back(RowVectorXd::Zero(layerValues[i]));
        }
    }
}

double MultiLayerNN::predict(RowVectorXd input) {
    if (input.size() != layers[0]) {
        return -1;
    }
    vector<RowVectorXd> adjustedLayers;
    for (int j = 0; j < layers.size(); j++) {
        if (j == 0) adjustedLayers.push_back(input);
        else if (j == 1) adjustedLayers.push_back(input * weights[0] + bias[0]);
        else adjustedLayers.push_back(adjustedLayers[j-1] * weights[j-1] + bias[j-1]);
        
        if (j != layers.size() - 1) {
            adjustedLayers[j] = adjustedLayers[j].unaryExpr(&relu);
        }
    }
    VectorXd outputLayer = adjustedLayers[layers.size() - 1];
    
    double max = 0, maxIndex = 0;
    for (int i = 0; i < outputLayer.size(); i++) {
        if (outputLayer(i) > max) {
            max = outputLayer(i);
            maxIndex = i;
        }
    }
    return maxIndex;
}

void MultiLayerNN::batchTrain(int iterations, MatrixXd input, VectorXd labels) {
    for (int i = 0; i < iterations; i++) {
        //feed forward
        int numWeights = (int) layers.size() - 1;
        vector<MatrixXd> adjustedLayers;
        for (int j = 0; j < layers.size(); j++) {
            if (j == 0) adjustedLayers.push_back(input);
            else {
                adjustedLayers.push_back(adjustedLayers[j-1] * weights[j-1] + bias[j-1].replicate(adjustedLayers[j-1].rows(), 1));
                if (j != numWeights) adjustedLayers[j] = adjustedLayers[j].unaryExpr(&relu);
            }
        }
        //normalize results
        MatrixXd expScores = adjustedLayers[numWeights].unaryExpr(&customExp);
        expScores = divideRowwiseSum(expScores);
        
        //get loss
        MatrixXd logProbs = computeLogProbs(expScores, labels);
        double dataLoss = logProbs.sum() / labels.size();
        double regLoss = 0;
        for (int j = 0; j < numWeights; j++) regLoss += 0.5 * reg * weights[j].unaryExpr(&sq).sum();
        double loss = dataLoss + regLoss;
        if (i % 100 == 0)
            cout << "iteration: " << i << ", loss: " << loss << "\n";
        
        if (i % 1000 == 0) {
            double accuracy = testAccuracy(input, labels);
            cout << "Accuracy: " << accuracy * 100 << "%\n";
            currentLoss = loss;
            currentAccuracy = accuracy;
            if (resavingData) save();
        }
        
        //calculate gradient descents
        vector<MatrixXd> gdScores;
        for (int j = 0; j < numWeights; j++) {
            if (j == 0) gdScores.push_back(computeGradientDescent(expScores,labels));
            else {
                gdScores.push_back(gdScores[j-1] * weights[numWeights - j].transpose());
                for (int k = 0; k < gdScores[j].rows(); k++) {
                    for (int l = 0; l < gdScores[j].cols(); l++) {
                        gdScores[j](k,l) = adjustedLayers[numWeights - j](k,l) <= 0 ? 0 : gdScores[j](k,l);
                    }
                }
            }
        }
        //adjust the weights
        for (int j = 0; j < numWeights; j++) {
            MatrixXd dW = adjustedLayers[numWeights-j-1].transpose() * gdScores[j];
            MatrixXd db = gdScores[j].colwise().sum();
            //cout << gdScores[j] << "\n";
            dW += reg * weights[numWeights-j-1];
            weights[numWeights-j-1] += -stepSize * dW;
            bias[numWeights-j-1] += -stepSize * db;
        }
    }
    
    //accuracy calculation
    currentAccuracy = testAccuracy(input, labels);
    cout << "Accuracy: " << currentAccuracy * 100 << "%\n";
}

double MultiLayerNN::testAccuracy(MatrixXd inputs, VectorXd labels) {
    //accuracy calculation
    vector<MatrixXd> adjustedLayers;
    for (int j = 0; j < layers.size(); j++) {
        if (j == 0) adjustedLayers.push_back(inputs);
        else if (j == 1) adjustedLayers.push_back(inputs * weights[0] + bias[0].replicate(inputs.rows(), 1));
        else adjustedLayers.push_back(adjustedLayers[j-1] * weights[j-1] + bias[j-1].replicate(adjustedLayers[j-1].rows(), 1));
        
        if (j != layers.size() - 1) {
            adjustedLayers[j] = adjustedLayers[j].unaryExpr(&relu);
        }
    }
    MatrixXd outputLayer = adjustedLayers[layers.size() - 1];
    
    RowVectorXd predictions(inputs.rows());
    
    for (int i = 0; i < outputLayer.rows(); i++) {
        int max = 0, maxIndex = 0;
        for (int j = 0; j < outputLayer.cols(); j++) {
            if (outputLayer(i,j) > max) {
                max = outputLayer(i,j);
                maxIndex = j;
            }
        }
        predictions(i) = maxIndex;
    }
    
    double sum = 0;
    for (int i = 0; i < inputs.rows(); i++) {
        if (predictions(i) == labels(i)) {
            sum++;
        }
    }
    
    return sum / labels.size();
}

void MultiLayerNN::save(string filename) {
    ofstream file(filename);
    
    string layersStr = "";
    for (int i = 0; i < layers.size(); i++) {
        layersStr += to_string(layers[i]);
        if (i != layers.size() - 1) layersStr += " ";
    }
    
    file << "Neural Network with [" << layers.size() << "] layers: " << layersStr <<"\n";
    file << "Loss: " << currentLoss << ", Accuracy: " << currentAccuracy * 100 << "%, stepSize: " << stepSize << ", reg: " << reg << "\n";
    file << "Weights: \n";
    for (int i = 0; i < weights.size(); i++) {
        for (int j = 0; j < weights[i].rows(); j++) {
            for (int k = 0; k < weights[i].cols(); k++) {
                file << weights[i](j,k);
                if (k != weights[i].cols() - 1) file << " ";
            }
            file << "\n";
        }
        file << "\n";
    }
    file << "Bias: \n";
    for (int i = 0; i < bias.size(); i++) {
        for (int j = 0; j < bias[i].size(); j++) {
            file << bias[i](j);
            if (j != bias[i].size() - 1) file << " ";
        }
        file << "\n\n";
    }
    file << "End File";
    file.close();
}
void MultiLayerNN::save() {
    save(saveFile);
}

void MultiLayerNN::setFile(string filename, bool saveData) {
    saveFile = filename;
    resavingData = saveData;
}

MatrixXd MultiLayerNN::divideRowwiseSum(MatrixXd m) {
    MatrixXd n = m.rowwise().sum();
    for (int i = 0; i < m.rows(); i++) {
        for (int j = 0; j < m.cols(); j++) {
            m(i,j) /= n(i,0);
        }
    }
    return m;
}

MatrixXd MultiLayerNN::computeLogProbs(MatrixXd m, VectorXd labels) {
    MatrixXd returnMatrix(m.rows(), 1);
    for (int i = 0; i < m.rows(); i++) {
        returnMatrix(i,0) = -log(m(i, labels(i)));
    }
    return returnMatrix;
}

MatrixXd MultiLayerNN::computeGradientDescent(MatrixXd m, VectorXd labels) {
    for (int i = 0; i < m.rows(); i++) m(i, labels(i)) -= 1;
    return m / labels.size();
}
