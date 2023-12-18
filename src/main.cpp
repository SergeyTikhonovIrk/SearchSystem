#include <algorithm>
#include <iostream>
#include <set>
#include <string>
#include <utility>
#include <vector>

using namespace std;

int MAX_RESULT_DOCUMENT_COUNT = 5;

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

set<string> ParseStopWords(const string &text)
{
  set<string> stop_words;
  for (const string &word : SplitIntoWords(text))
  {
    stop_words.insert(word);
  }
  return stop_words;
}

vector<string> SplitIntoWordsNoStop(const string &text,
                                    const set<string> &stop_words)
{
  vector<string> words;
  // проходим по всем словам из текста и проверяем, что их нет в списке
  // стоп-слов
  for (const string &word : SplitIntoWords(text))
  {
    if (stop_words.count(word) == 0)
    {
      words.push_back(word);
    }
  }
  // вернём результат без стоп-слов
  return words;
}

void AddDocument(vector<pair<int, vector<string>>> &documents,
                 const set<string> &stop_words, int document_id,
                 const string &document)
{
  const vector<string> words = SplitIntoWordsNoStop(document, stop_words);
  documents.push_back({document_id, words});
}

// Разбирает text на слова и возвращает только те из них, которые не входят в
// stop_words
set<string> ParseQuery(const string &text, const set<string> &stop_words)
{
  const auto query_words = SplitIntoWordsNoStop(text, stop_words);
  return {query_words.begin(), query_words.end()};
}

// Возвращает релевантность документа
int MatchDocument(const pair<int, vector<string>> &content,
                  const set<string> &query_words)
{
  set<string> doc_words{content.second.begin(), content.second.end()};
  vector<string> s_intersection; // пересечение множеств слов документа и слов запроса
  set_intersection(doc_words.begin(), doc_words.end(), query_words.begin(), query_words.end(), back_inserter(s_intersection));
  return s_intersection.size();
}

// Для каждого документа возвращает его релевантность и id
vector<pair<int, int>> FindAllDocuments(const vector<pair<int, vector<string>>> &documents, const set<string> &query_words)
{
  vector<pair<int, int>> matched_documents;
  for (size_t idx = 0; idx < documents.size(); ++idx)
  {
    if (int relevance = MatchDocument(documents[idx], query_words); relevance > 0)
    {
      matched_documents.push_back({idx, relevance});
    }
  }
  return matched_documents;
}

// Возвращает топ-5 самых релевантных документов в виде пар: {id, релевантность}
vector<pair<int, int>> FindTopDocuments(const vector<pair<int, vector<string>>> &documents, const set<string> &stop_words, const string &raw_query)
{
  vector<pair<int, int>> matched_documents = FindAllDocuments(documents, ParseQuery(raw_query, stop_words));
  sort(matched_documents.begin(), matched_documents.end());
  reverse(matched_documents.begin(), matched_documents.end());
  if (matched_documents.size() > MAX_RESULT_DOCUMENT_COUNT)
  {
    matched_documents.resize(MAX_RESULT_DOCUMENT_COUNT);
  }
  // Массив matched_documents содержит пары {релевантность, id}
  // FindTopDocuments должна вернуть пары {id, релевантность}
  // Поэтому нужно обменять элементы пар, перед тем как вернуть результат
  for (auto &matched_document : matched_documents)
  {
    swap(matched_document.first, matched_document.second);
  }
  return matched_documents;
}

int main()
{
  const string stop_words_joined = ReadLine();
  const set<string> stop_words = ParseStopWords(stop_words_joined);

  // Read documents
  vector<pair<int, vector<string>>> documents;
  const int document_count = ReadLineWithNumber();
  for (int document_id = 0; document_id < document_count; ++document_id)
  {
    AddDocument(documents, stop_words, document_id, ReadLine());
  }

  const string query = ReadLine();

  for (auto [document_id, relevance] : FindTopDocuments(documents, stop_words, query))
  {
    cout << "{ document_id = "s << document_id << ", relevance = "s << relevance << " }"s
         << endl;
  }
}