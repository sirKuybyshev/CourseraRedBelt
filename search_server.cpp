#include "search_server.h"
#include "iterator_range.h"

#include <algorithm>
#include <iostream>
#include <iterator>
#include <sstream>

vector<string> SplitIntoWords(const string &line) {
    istringstream words_input(line);
    return {istream_iterator<string>(words_input), istream_iterator<string>()};
}

void doUpdate(istream &document_input, Synchronized<InvertedIndex> &oldIndex) {
    InvertedIndex new_index;
    for (string current_document; getline(document_input, current_document);) {
        new_index.Add(move(current_document));
    }
    auto index = oldIndex.GetAccess();
    index.ref_to_value = move(new_index);
}

SearchServer::SearchServer(istream &document_input) {
    doUpdate(document_input, index);
}

void SearchServer::UpdateDocumentBase(istream &document_input) {
    futures.push_back(async(doUpdate, ref(document_input), ref(index)));
}


template<typename BidIter>
void isTopFive(BidIter begin, BidIter end, int &first, int &second) {
    auto bound = end;
    bound--;
    bool check = false;
    while (bound >= begin && (*bound).second < second) {
        bound--;
        check = true;
    }
    if (check) {
        begin = ++bound;
        auto temp = *bound++;
        while (bound != end) {
            auto temp1 = *bound;
            *bound++ = temp;
            temp = temp1;
        }
        *begin = {first, second};
    }
}

void SearchServer::AddQueriesOneThread(istream &query_input, ostream &search_results_output) {
    vector<int> docid_count;
    docid_count.reserve(50'000);
    for (string current_query; getline(query_input, current_query);) {
        const auto words = SplitIntoWords(current_query);
        for (const auto &word : words) {
            auto syncIndex = index.GetAccess();
            for (const auto &item : syncIndex.ref_to_value.Lookup(word)) {
                if (item.first + 1 > docid_count.size()) {
                    docid_count.resize(min(item.first + 10, 50'000));
                }
                docid_count[item.first] += item.second;
            }
        }
        vector<pair<int, int>> search_results(5, {0, 0});
        for (int i = 0; i < docid_count.size(); ++i) {
            isTopFive(search_results.begin(), search_results.end(), i, docid_count[i]);
        }
        search_results_output << current_query << ':';
        for (auto [docid, hitcount] : Head(search_results, 5)) {
            if (hitcount == 0) {
                continue;
            }
            search_results_output << " {"
                                  << "docid: " << docid << ", "
                                  << "hitcount: " << hitcount << '}';
        }
        search_results_output << endl;
        docid_count.clear();
    }
}

void SearchServer::AddQueriesStream(
        istream &query_input, ostream &search_results_output) {
    futures.push_back(async(&SearchServer::AddQueriesOneThread, this, ref(query_input), ref(search_results_output)));
}

void InvertedIndex::Add(string document) {
    const size_t docid = docs.size();
    for (const auto &word : SplitIntoWords(document)) {
        auto place = index.find(word);
        if (place != index.end()) {
            if (docid == place->second.back().first) {
                place->second.back().second++;
            } else {
                place->second.emplace_back(docid, 1);
            }
        } else {
            index[word].emplace_back(docid, 1);
        }
    }
    docs.push_back(move(document));
}

const vector<pair<int, int>> &InvertedIndex::Lookup(const string &word) const {
    if (auto it = index.find(word); it != index.end()) {
        return it->second;
    } else {
        return nullVec;
    }
}
