#include <iostream>
#include <fstream>
#include <sstream>
#include <algorithm>
#include <string>
#include <vector>
#include <set>
#include <map>
#include <chrono>

#define DATA_FILE_NAME "C:\\Users\\kirpa\\CLionProjects\\AlgorithmsAI\\OnlineRetail.csv"
#define SHOPPING_ID_FIELD_NAME "InvoiceNo"
#define ITEM_ID_FIELD_NAME "StockCode"
#define MIN_N_THRESHOLD 100

using namespace std;


class FPNode {
public:
    string value;
    long long freq = 0;

    map<string, FPNode> children;

    FPNode() = default;

    FPNode(string value, long long freq): value(move(value)), freq(freq) {
    }

    void add_child(const FPNode& child_node) {
        children[child_node.value] = child_node;
    }
};


void read_data(vector<map<string, string> >& data) {
    fstream fin;

    fin.open(DATA_FILE_NAME, ios::in);

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


void add_elements(FPNode& source, const vector<string>& added_elements, int added_i) {
    string added = added_elements[added_i];
    FPNode added_node = source.children[added];
    added_node.value = added;
    added_node.freq += 1;
    if(added_i < added_elements.size() - 1) {
        add_elements(added_node, added_elements, added_i + 1);
    }
    source.add_child(added_node);
}


int main() {
    chrono::steady_clock::time_point begin = chrono::steady_clock::now();

    vector<map<string, string> > data;

    cout << "Reading data from " << DATA_FILE_NAME << endl;
    read_data(data);
    cout << "Successfully read " << data.size() << " lines of data" << endl << endl;

    cout << "Preparing itemsets for each transaction" << endl;
    map<string, set<string> > itemsets;
    for(auto el: data) {
        string curr_shopping_id = el[SHOPPING_ID_FIELD_NAME];
        itemsets[curr_shopping_id].insert(el[ITEM_ID_FIELD_NAME]);
    }
    cout << "Itemsets are ready. The number of shoppings is " << itemsets.size() << endl << endl;

    cout << "Counting items entries" << endl;
    map<string, long long> count_entries;
    for(const auto& itemset: itemsets) {
        for(const auto& item: itemset.second) {
            count_entries[item] += 1;
        }
    }
    cout << "Successfully counted " << count_entries.size() << " items" << endl << endl;

    cout << "Building FP-tree" << endl;
    map<string, FPNode> fp_tree;
    for(const auto& itemset: itemsets) {
        vector<string> frequent_items;
        for(const auto& item: itemset.second) {
            if(count_entries[item] >= MIN_N_THRESHOLD) {
                frequent_items.push_back(item);
            }
        }
        if(frequent_items.empty()) continue;
        sort(frequent_items.begin(), frequent_items.end(),
             [&count_entries](const string& a, const string& b) -> bool {
                 return count_entries[a] > count_entries[b];
             }
        );
        string root_value = frequent_items[0];
        FPNode root_node = fp_tree[root_value];
        root_node.value = root_value;
        root_node.freq += 1;
        if(frequent_items.size() > 1) {
            add_elements(root_node, frequent_items, 1);
        }
        fp_tree[root_value] = root_node;
    }
    cout << "Successfully built FP-tree" << endl << endl;


    chrono::steady_clock::time_point end = chrono::steady_clock::now();
    cout << "Preparing FP algorithm took " << chrono::duration_cast<chrono::seconds>(end - begin).count() << "s" << endl << endl;


    vector<string> checked_set;
    cout << "Input the set:" << endl;
    string user_input, word_buffer;
    cin >> user_input;
    stringstream user_stream(user_input);
    while(getline(user_stream, word_buffer, ' ')) {
        checked_set.push_back(word_buffer);
    }
    sort(checked_set.begin(), checked_set.end(),
         [&count_entries](const string& a, const string& b) -> bool {
             return count_entries[a] > count_entries[b];
         }
    );
    string first_item = checked_set[0];
    if (fp_tree.find(first_item) == fp_tree.end()) {
        cout << "Nothing found" << endl;
    }
    else {
        FPNode last_node = fp_tree[first_item];
        bool found = true;
        for (int user_item_i = 1; user_item_i < checked_set.size(); user_item_i++) {
            string current_item = checked_set[user_item_i];
            cout << current_item << endl;
            if (last_node.children.find(current_item) == last_node.children.end()) {
                cout << "Nothing found" << endl;
                found = false;
                break;
            }
            last_node = last_node.children[current_item];
        }
        if (found) {
            vector<pair<string, int> > candidates;
            for(const auto& candidate: last_node.children) {
                candidates.emplace_back(candidate.first, candidate.second.freq);
            }
            sort(candidates.begin(), candidates.end(),
                 [](const pair<string, int>& a, const pair<string, int>& b) -> bool {
                     return a.second > b.second;
                 }
            );
            cout << "Recommendation:" << endl;
            for(const auto& candidate: candidates) {
                cout << candidate.first << ' ';
            }
        }
    }

    return 0;
}  // 85123A
