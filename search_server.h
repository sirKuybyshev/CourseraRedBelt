#pragma once

#include "synchronized.h"

#include <future>
#include <istream>
#include <list>
#include <map>
#include <ostream>
#include <set>
#include <string>
#include <vector>
using namespace std;

class InvertedIndex {
public:
    void Add(string document);
    const vector<pair<int, int>> &Lookup(const string &word) const;

    const string &GetDocument(size_t id) const {
        return docs[id];
    }



private:
    vector<string> docs;
    map<string, vector<pair<int, int>>> index;
    vector<pair<int, int>> nullVec;
};

class SearchServer {
public:
    SearchServer() = default;
    explicit SearchServer(istream &document_input);
    void UpdateDocumentBase(istream &document_input);
    void AddQueriesStream(istream &query_input, ostream &search_results_output);
    void AddQueriesOneThread(istream &query_input, ostream &search_results_output);

    ~SearchServer() {
        for (auto &item : futures) {
            item.get();
        }
    }

private:
    Synchronized<InvertedIndex> index;
    vector<future<void>> futures;
};
