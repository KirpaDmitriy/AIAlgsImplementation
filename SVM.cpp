#include <iostream>
#include <fstream>
#include <sstream>
#include <algorithm>
#include <string>
#include <vector>
#include <map>
#include <chrono>
#include <random>

#define TRAIN_DATA_FILE_NAME "C:\\Users\\kirpa\\CLionProjects\\AlgorithmsAI\\SVMDataTrain.csv"
#define TEST_DATA_FILE_NAME "C:\\Users\\kirpa\\CLionProjects\\AlgorithmsAI\\SVMDataTest.csv"
#define TARGET_FIELD_NAME "Part"

using namespace std;


void read_data(vector<map<string, string> >& data, const string& file_name) {
    fstream fin;

    fin.open(file_name, ios::in);

    vector<string> row;
    string line, word, temp;

    vector<string> index_to_column;
    bool is_column_line = true;

    while(fin >> temp) {
        row.clear();

        getline(fin, line);
        stringstream s(temp + line);
        while (getline(s, word, ',')) {
            row.push_back(word);
        }
        if(row.empty()) continue;

        if(is_column_line) {
            for (const auto& column_name: row) {
                index_to_column.push_back(column_name);
            }
        } else {
            map<string, string> current_line;
            for(int i = 0; i < row.size(); i++) {
                current_line[index_to_column[i]] = row[i];
            }
            data.push_back(current_line);
        }

        is_column_line = false;
    }
}

double scalar_vector_multiplication(const vector<double>& a, const vector<double>& b) {
    double result = 0;
    for(int i=0; i<a.size(); i++) {
        result += a[i] * b[i];
    }
    return result;
}

void vector_multiplication(vector<double>& source, const vector<double>& multiplied) {
    for(int i=0; i<source.size(); i++) {
        source[i] *= multiplied[i];
    }
}

void vector_difference(vector<double>& source, const vector<double>& deductible) {
    for(int i=0; i<source.size(); i++) {
        source[i] -= deductible[i];
    }
}

double hinge_loss(const vector<double>& x, double y, const vector<double>& w) {
    vector<double> tmp(x.size(), y);
    vector_multiplication(tmp, x);
    return fmax(0, 1 - scalar_vector_multiplication(tmp, w));
}


class SVM {
    vector<double> weights;
    vector<vector<double> > learning_history;
    vector<double> losses;

    void init_weights(unsigned int n_dimensions) {
        weights.clear();
        weights.resize(n_dimensions + 1);
        for(int weight_i=0; weight_i < n_dimensions + 1; weight_i++) {
            double init_w_i = (double)rand() / (double)RAND_MAX;
            weights[weight_i] = init_w_i;
        }
    }

public:
    void fit(const vector<map<string, string> >& data, const string& target_name, double grad_step, double alpha, unsigned int epochs) {
        learning_history.clear();
        losses.clear();

        init_weights(data[0].size());
        for(int epoch_n=0; epoch_n < epochs; epoch_n++) {
            double loss = 0;
            for (auto line: data) {
                int target = stoi(line[target_name]);
                vector<double> x(0);
                for (const auto &val: line) {
                    if (val.first == target_name) {
                        continue;
                    }
                    x.push_back(stod(val.second));
                }
                x.push_back(1);
                double prediction_correctness = scalar_vector_multiplication(x, weights) * target;
                if(prediction_correctness >= 1) {
                    vector<double> tmp(weights.size(), grad_step * alpha / epochs);
                    vector_multiplication(tmp, weights);
                    vector_difference(weights, tmp);
                }
                else {
                    vector<double> tmp(weights.size(), alpha / epochs);
                    vector_multiplication(tmp, weights);
                    vector<double> tmp1(x.size(), target);
                    vector_multiplication(tmp1, x);
                    vector_difference(tmp, tmp1);
                    vector_difference(weights, tmp);
                }
                loss += hinge_loss(x, target, weights);
            }
            learning_history.push_back(weights);
            loss /= data.size();
            losses.push_back(loss);
            cout << "Epoch " << epoch_n << " loss is " << loss << endl;
        }
    }
};


int main() {
    chrono::steady_clock::time_point begin = chrono::steady_clock::now();

    vector<map<string, string> > train_data;
    vector<map<string, string> > test_data;

    cout << "Reading train data from " << TRAIN_DATA_FILE_NAME << endl;
    read_data(train_data, TRAIN_DATA_FILE_NAME);
    cout << "Successfully read " << train_data.size() << " lines of train data" << endl << endl;

    cout << "Reading test data from " << TEST_DATA_FILE_NAME << endl;
    read_data(test_data, TEST_DATA_FILE_NAME);
    cout << "Successfully read " << test_data.size() << " lines of test data" << endl << endl;

    SVM svm;

    svm.fit(train_data, TARGET_FIELD_NAME, 0.5, 0.3, 10000);

    return 0;
}
