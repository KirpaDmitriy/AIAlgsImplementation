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
#define MIN_PROBA_THRESHOLD 0.029

using namespace std;


class AprioryNode;

class AprioryVertex {
public:
    AprioryNode* parent;
    AprioryNode* child;
    double weight;

    AprioryVertex() = default;

    AprioryVertex(AprioryNode* parent, AprioryNode* child, double weight):
            parent(parent), child(child), weight(weight) {}
};

class AprioryNode {
public:
    set<string> value;
    long long freq;

    map<set<string>, AprioryVertex> connections;

    AprioryNode() {
        value = set<string>();
        freq = 0;
    }

    AprioryNode(set<string> value, long long freq): value(move(value)), freq(freq) {
    }

    void add_connection(const set<string>& child_val, AprioryVertex new_vertex) {
        connections[child_val] = new_vertex;
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


bool is_frequent(long long n_entries, long long itemsets_size) {
    return (double)n_entries / itemsets_size >= MIN_PROBA_THRESHOLD;
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

    cout << "Calculating the number of items entries" << endl;
    map<string, long long> count_base;
    for(const auto& itemset: itemsets) {
        for(const auto& item: itemset.second) {
            count_base[item] += 1;
        }
    }
    cout << "Calculated the number of entries for " << count_base.size() << " items" << endl << endl;

    cout << "Building the sets of transactions for each item" << endl;
    map<set<string>, set<string> > items_transactions;
    for(auto& el: data) {
        string item_id = el[ITEM_ID_FIELD_NAME];
        if(is_frequent(count_base[item_id], itemsets.size())) {
            items_transactions[set<string>{item_id}].insert(el[SHOPPING_ID_FIELD_NAME]);
        }
    }
    cout << "Successfully built the sets of transactions for " << items_transactions.size() << " items" << endl << endl;

    data.clear();
    count_base.clear();

    cout << "Preparing the first level of the apriory tree" << endl;
    vector<map<set<string>, AprioryNode> > apriory_tree;
    apriory_tree.emplace_back();
    for(const auto& item_transactions: items_transactions) {
        set<string> curr_value = {item_transactions.first};
        apriory_tree[0][curr_value] = AprioryNode(curr_value, item_transactions.second.size());
    }
    cout << "The first layer is done" << endl << endl;

    unsigned int last_level_n = apriory_tree.size() - 1;
    while(!apriory_tree[last_level_n].empty()) {
        cout << "Making " << last_level_n + 2 << "-elements sets of frequent elements" << endl;
        apriory_tree.emplace_back();
        map<set<string>, set<string> > prev_level_transactions = items_transactions;
        cout << "Processing " << apriory_tree[last_level_n].size() << " previous level elements " <<
             apriory_tree[0].size() << " base items per each level" << endl;
        unsigned int iter_counter = 1;
        for (const auto &prev_level_node: apriory_tree[last_level_n]) {
            iter_counter++;
            set<string> prev_level_items = prev_level_node.first;
            for (const auto &added_new: apriory_tree[0]) {
                string added_new_val;
                for(const string& added: added_new.first) {
                    added_new_val = added;
                }
                if(prev_level_items.find(added_new_val) != prev_level_items.end()) {
                    continue;
                }
                set<string> new_level_items = prev_level_items;
                new_level_items.insert(added_new.first.begin(), added_new.first.end());
                set<string> prev_level_el_transactions = prev_level_transactions[prev_level_items];
                set<string> added_el_transactions = items_transactions[added_new.first];
                set<string> intersection;
                set_intersection(prev_level_el_transactions.begin(), prev_level_el_transactions.end(),
                                 added_el_transactions.begin(), added_el_transactions.end(),
                                 std::inserter(intersection, intersection.begin()));
                long long intersection_size = intersection.size();
                if (is_frequent(intersection_size, itemsets.size())) {
                    apriory_tree[last_level_n + 1][new_level_items] = AprioryNode(new_level_items,
                                                                                  intersection_size);
                    prev_level_transactions[new_level_items] = intersection;
                    double connection_weight = (double) intersection_size / prev_level_el_transactions.size();
                    AprioryVertex interlevel_connection = AprioryVertex((AprioryNode *) &prev_level_node,
                                                                        (AprioryNode *) &apriory_tree[last_level_n +
                                                                                                      1][new_level_items],
                                                                        connection_weight);
                    apriory_tree[last_level_n][prev_level_items].add_connection(new_level_items,
                                                                                interlevel_connection);
                }
            }
            prev_level_transactions.erase(prev_level_items);
        }
        last_level_n = apriory_tree.size() - 1;
        cout << "Layer building finished" << endl;
        cout << "--------------------" << endl;
    }

    cout << "Prepared tree of size " << apriory_tree.size() << endl << endl;

    chrono::steady_clock::time_point end = chrono::steady_clock::now();
    cout << "Preparing Apriory algorithm took " << chrono::duration_cast<chrono::seconds>(end - begin).count() << "s" << endl << endl;


    set<string> checked_set;
    cout << "Input the set:" << endl;
    string user_input, word_buffer;
    cin >> user_input;
    stringstream user_stream(user_input);
    while(getline(user_stream, word_buffer, ' ')) {
        checked_set.insert(word_buffer);
    }
    unsigned int input_level = checked_set.size() - 1;
    cout << "Searching on the level " << input_level << endl;
    if(apriory_tree[input_level].find(checked_set) == apriory_tree[input_level].end()) {
        cout << "Nothing found" << endl;
    }
    else {
        AprioryNode user_node = apriory_tree[input_level][checked_set];
        map<set<string>, AprioryVertex> search_base = user_node.connections;
        double max_proba = 0;
        set<string> max_proba_node;
        for(const auto& search_el: search_base) {
            double current_vertex_weight = search_el.second.weight;
            if(max_proba < current_vertex_weight) {
                max_proba_node = search_el.first;
                max_proba = current_vertex_weight;
            }
        }

        set<string> recommendation;
        set_difference(max_proba_node.begin(), max_proba_node.end(),
                       checked_set.begin(), checked_set.end(),
                       std::inserter(recommendation, recommendation.begin()));
        for(const auto& recommended_element: recommendation) {
            cout << recommended_element << ' ';
        }
        cout << endl;
    }

    return 0;
}
