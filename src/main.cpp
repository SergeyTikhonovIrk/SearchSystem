#include <algorithm>
#include <iostream>
#include <set>
#include <string>
#include <utility>
#include <vector>
#include <map>
#include <cmath>
#include <sstream>
#include <numeric>

using namespace std;

const int MAX_RESULT_DOCUMENT_COUNT = 5;

string ReadLine()
{
    string s;
    getline(cin, s);
    return s;
}

int ReadLineWithNumber()
{
    int result = 0;
    cin >> result;
    ReadLine();
    return result;
}

vector<int> ReadRating()
{
    string line;
    getline(cin, line);
    istringstream in(line);
    vector<int> result;
    int number;
    while (in >> number)
    {
        result.push_back(number);
    }
    return result;
}

vector<string> SplitIntoWords(const string &text)
{
    vector<string> words;
    string word;
    for (const char c : text)
    {
        if (c == ' ')
        {
            if (!word.empty())
            {
                words.push_back(word);
                word.clear();
            }
        }
        else
        {
            word += c;
        }
    }
    if (!word.empty())
    {
        words.push_back(word);
    }

    return words;
}

struct Document
{
    int id;
    double relevance;
    int rating;
};

class SearchServer
{
public:
    void SetStopWords(const string &text)
    {
        for (const string &word : SplitIntoWords(text))
        {
            stop_words_.insert(word);
        }
    }

    void AddDocument(int document_id, const string &document, const vector<int> &ratings)
    {
        const auto &words = SplitIntoWordsNoStop(document);               
        for (const string &word : words)
        {
            word_to_document_freqs_[word][document_id] += 1 / static_cast<double>(words.size());
        }
        ratings_.emplace(document_id, ComputeAverageRating(ratings));
        ++document_count_;
    }

    vector<Document> FindTopDocuments(const string &raw_query) const
    {
        const Query query_words = ParseQuery(raw_query);
        auto matched_documents = FindAllDocuments(query_words);

        sort(matched_documents.begin(), matched_documents.end(),
             [](const Document &lhs, const Document &rhs)
             {
                 return lhs.relevance > rhs.relevance;
             });
        if (matched_documents.size() > MAX_RESULT_DOCUMENT_COUNT)
        {
            matched_documents.resize(MAX_RESULT_DOCUMENT_COUNT);
        }
        return matched_documents;
    }

private:
    int document_count_ = 0;
    struct Query
    {
        set<string> plus_words_;
        set<string> minus_words_;
    };

    map<string, map<int, double>> word_to_document_freqs_;

    map<int,int> ratings_;

    set<string> stop_words_;

    bool IsStopWord(const string &word) const
    {
        return stop_words_.count(word) > 0;
    }

    bool IsMinusWord(const string &word) const
    {
        return word[0] == '-';
    }

    vector<string> SplitIntoWordsNoStop(const string &text) const
    {
        vector<string> words;
        for (const string &word : SplitIntoWords(text))
        {
            if (!IsStopWord(word))
            {
                words.push_back(word);
            }
        }
        return words;
    }

    Query ParseQuery(const string &text) const
    {
        Query words;
        for (const string &word : SplitIntoWordsNoStop(text))
        {
            IsMinusWord(word) ? words.minus_words_.insert(word.substr(1, word.size())) : words.plus_words_.insert(word);
        }
        return words;
    }

    const double ComputeWordIDF(const string &word) const
    {
        return log(document_count_ / static_cast<double>(word_to_document_freqs_.at(word).size()));
    }

    vector<Document> FindAllDocuments(const Query &query_words) const
    {
        map<int, double> id_relevance;
        vector<Document> matched_documents;
        for (const auto &word : query_words.plus_words_)
        {
            if (word_to_document_freqs_.count(word) == 1)
            {
                for (const auto &[id, tf] : word_to_document_freqs_.at(word))
                {
                    id_relevance[id] += (ComputeWordIDF(word) * tf);
                }
            }
        }
        for (const auto &word : query_words.minus_words_)
        {
            if (word_to_document_freqs_.count(word) == 1)
            {
                for (const auto &[id, _] : word_to_document_freqs_.at(word))
                {
                    id_relevance.erase(id);
                }
            }
        }

        for (const auto &[id, relevance] : id_relevance)
        {
            matched_documents.push_back({id, relevance, ratings_.at(id)});
        }
        return matched_documents;
    }

    static int ComputeAverageRating(const vector<int>& ratings) {
        if (ratings.empty()) {
            return 0;
        }
        int rating_sum = 0;
        for (const int rating : ratings) {
            rating_sum += rating;
        }
        return rating_sum / static_cast<int>(ratings.size());
    }
};


SearchServer CreateSearchServer() {
    SearchServer search_server;
    search_server.SetStopWords(ReadLine());

    const int document_count = ReadLineWithNumber();
    for (int document_id = 0; document_id < document_count; ++document_id) {
        const string document = ReadLine();
        int ratings_size;
        cin >> ratings_size;
        
        // создали вектор размера ratings_size из нулей
        vector<int> ratings(ratings_size, 0);
        
        // считали каждый элемент с помощью ссылки
        for (int& rating : ratings) {
            cin >> rating;
        }
        
        search_server.AddDocument(document_id, document, ratings);
        ReadLine();
    }
    
    return search_server;
}


int main() {
    const SearchServer search_server = CreateSearchServer();

    const string query = ReadLine();
    for (const Document& document : search_server.FindTopDocuments(query)) {
        cout << "{ "
             << "document_id = " << document.id << ", "
             << "relevance = " << document.relevance << ", "
             << "rating = " << document.rating
             << " }" << endl;
    }
}