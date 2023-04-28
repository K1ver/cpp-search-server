#include <algorithm>
#include <cmath>
#include <map>
#include <set>
#include <string>
#include <utility>
#include <vector>

#include "search_server.h"

using namespace std;

/* Подставьте вашу реализацию класса SearchServer сюда */

/*
   Подставьте сюда вашу реализацию макросов
   ASSERT, ASSERT_EQUAL, ASSERT_EQUAL_HINT, ASSERT_HINT и RUN_TEST
*/

template <typename T, typename U>
void AssertEqualImpl(const T& t, const U& u, const string& t_str, const string& u_str, const string& file,
    const string& func, unsigned line, const string& hint) {
    if (t != u) {
        cerr << boolalpha;
        cerr << file << "("s << line << "): "s << func << ": "s;
        cerr << "ASSERT_EQUAL("s << t_str << ", "s << u_str << ") failed: "s;
        cerr << t << " != "s << u << "."s;
        if (!hint.empty()) {
            cerr << " Hint: "s << hint;
        }
        cerr << endl;
        abort();
    }
}
 
void AssertImpl(bool value, const string& expr_str, const string& file, const string& func, unsigned line,
    const string& hint) {
    if (!value) {
        cerr << file << "("s << line << "): "s << func << ": "s;
        cerr << "ASSERT("s << expr_str << ") failed."s;
        if (!hint.empty()) {
            cerr << " Hint: "s << hint;
        }
        cerr << endl;
        abort();
    }
}

// -------- Начало модульных тестов поисковой системы ----------

// Тест проверяет, что поисковая система исключает стоп-слова при добавлении документов
void TestExcludeStopWordsFromAddedDocumentContent() {
    const int doc_id = 42;
    const string content = "cat in the city"s;
    const vector<int> ratings = {1, 2, 3};
    {
        SearchServer server;
        server.AddDocument(doc_id, content, DocumentStatus::ACTUAL, ratings);
        const auto found_docs = server.FindTopDocuments("in"s);
        ASSERT_EQUAL(found_docs.size(), 1u);
        const Document& doc0 = found_docs[0];
        ASSERT_EQUAL(doc0.id, doc_id);
    }

    {
        SearchServer server;
        server.SetStopWords("in the"s);
        server.AddDocument(doc_id, content, DocumentStatus::ACTUAL, ratings);
        ASSERT_HINT(server.FindTopDocuments("in"s).empty(),
                    "Stop words must be excluded from documents"s);
    }
}

void TestFindAddedDocumentByDocumentWord() {
    const int doc_id = 42;
    const string content = "cat in the city"s;
    const vector<int> ratings = { 1, 2, 3 };
 
    {
        SearchServer server;
        ASSERT_EQUAL(server.GetDocumentCount(), 0);
        server.AddDocument(doc_id, content, DocumentStatus::ACTUAL, ratings);
        ASSERT_EQUAL(server.GetDocumentCount(), 1);
        server.AddDocument(doc_id + 1, "black dog fluffy tail", DocumentStatus::ACTUAL, ratings);
        ASSERT_EQUAL(server.GetDocumentCount(), 2);
        const auto found_docs = server.FindTopDocuments("cat"s);
        ASSERT_EQUAL(found_docs.size(), 1u);
        const Document& doc0 = found_docs[0];
        ASSERT_EQUAL(doc0.id, doc_id);
    }
}

void TestExcludeDocumentsWithMinusWordsFromResults() {
    SearchServer server;
    server.AddDocument(101, "fluffy cat fluffy tail"s, DocumentStatus::ACTUAL, { 1,2,3 });
    server.AddDocument(102, "fluffy red dog "s, DocumentStatus::ACTUAL, { 1,2,3 });
 
    {
        const auto found_docs = server.FindTopDocuments("fluffy -dog"s);
        ASSERT_EQUAL(found_docs.size(), 1u);
        ASSERT_EQUAL(found_docs[0].id, 101);
    }
 
    {
        const auto found_docs = server.FindTopDocuments("fluffy -cat"s);
        ASSERT_EQUAL(found_docs.size(), 1u);
        ASSERT_EQUAL(found_docs[0].id, 102);
    }
 
}

void TestMatchedDocuments() {
    SearchServer server;
    server.SetStopWords("and in on"s);
    server.AddDocument(100, "fluffy cat and black dog in a collar"s, DocumentStatus::ACTUAL, { 1, 2, 3 });
 
    {
        const auto [matched_words, status] = server.MatchDocument("dog and cat"s, 100);
        const vector<string> expected_result = { "cat"s, "dog"s };
        ASSERT_EQUAL(expected_result, matched_words);
    }
 
    {
        const auto [matched_words, status] = server.MatchDocument("dog and -cat"s, 100);
        const vector<string> expected_result = {}; // пустой результат поскольку есть минус-слово 
        ASSERT_EQUAL(expected_result, matched_words);
        ASSERT(matched_words.empty());
    }
}

void TestSortResultsByRelevance() {
    SearchServer server;
    server.AddDocument(100, "fluffy cat fluffy tail"s, DocumentStatus::ACTUAL, { 1, 2, 3 });
    server.AddDocument(101, "fluffy dog"s, DocumentStatus::ACTUAL, { 1, 2, 3 });
    server.AddDocument(102, "dog leather collar"s, DocumentStatus::ACTUAL, { 1, 2, 3 });
 
    {
        const auto found_docs = server.FindTopDocuments("fluffy cat"s);
        ASSERT_EQUAL(found_docs.size(), 2u);
        for (size_t i = 1; i < found_docs.size(); i++) {
            ASSERT(found_docs[i - 1].relevance >= found_docs[i].relevance);
        }
    }
}

void TestCalculateDocumentRating() {
    SearchServer server;
    const vector<int> ratings = { 10, 11, 3 };
    const int average = (10 + 11 + 3) / 3;
    server.AddDocument(0, "fluffy cat fluffy tail"s, DocumentStatus::ACTUAL, ratings);
 
    {
        const auto found_docs = server.FindTopDocuments("fluffy cat"s);
        ASSERT_EQUAL(found_docs.size(), 1u);
        ASSERT_EQUAL(found_docs[0].rating, average);
    }
}

void TestDocumentSearchByPredicate() {
    SearchServer server;
    server.AddDocument(100, "cat in the city"s, DocumentStatus::ACTUAL, { 1, 2, 3 });
    server.AddDocument(101, "dog in the town"s, DocumentStatus::IRRELEVANT, { -1, -2, -3 });
    server.AddDocument(102, "dog and rabbit in the town"s, DocumentStatus::ACTUAL, { -4, -5, -6 });
    const auto found_docs = server.FindTopDocuments("in the cat"s, [](int document_id, DocumentStatus status, int rating) { return rating > 0; });
 
    {
        ASSERT(found_docs[0].id == 100);
    }
}

void TestDocumentSearchByStatus() {
    const int doc_id1 = 42;
    const int doc_id2 = 43;
    const int doc_id3 = 44;
    const string content1 = "cat in the city"s;
    const string content2 = "cat in the town"s;
    const string content3 = "cat in the town or city"s;
    const vector<int> ratings = { 1, 2, 3 };
    SearchServer server;
    server.AddDocument(doc_id1, content1, DocumentStatus::ACTUAL, ratings);
    server.AddDocument(doc_id2, content2, DocumentStatus::IRRELEVANT, ratings);
    server.AddDocument(doc_id3, content3, DocumentStatus::IRRELEVANT, ratings);
    const auto found_docs = server.FindTopDocuments("in the cat"s, DocumentStatus::IRRELEVANT);
 
    {
        ASSERT(found_docs[0].id == doc_id2);
        ASSERT(found_docs[1].id == doc_id3);
        ASSERT(found_docs.size() == 2);
    }
}

void TestCalculateRelevance() {
    SearchServer server;
    server.AddDocument(100, "white cat with new ring"s, DocumentStatus::ACTUAL, { 1, 2, 3 });
    server.AddDocument(101, "fluffy cat fluffy tail"s, DocumentStatus::ACTUAL, { 1, 2, 3 });
    server.AddDocument(102, "good dog big eyes"s, DocumentStatus::ACTUAL, { 1, 2, 3 });
    const auto found_docs = server.FindTopDocuments("fluffy good cat"s);
    double relevance_query = log((3 * 1.0) / 1) * (2.0 / 4.0) + log((3 * 1.0) / 2) * (1.0 / 4.0);
 
    {
        ASSERT(fabs(found_docs[0].relevance - relevance_query) < 1e-6);
    }
}
 
// Функция TestSearchServer является точкой входа для запуска тестов 
void TestSearchServer() {
    RUN_TEST(TestExcludeStopWordsFromAddedDocumentContent);
    RUN_TEST(TestFindAddedDocumentByDocumentWord);
    RUN_TEST(TestExcludeDocumentsWithMinusWordsFromResults);
    RUN_TEST(TestMatchedDocuments);
    RUN_TEST(TestSortResultsByRelevance);
    RUN_TEST(TestCalculateDocumentRating);
    RUN_TEST(TestDocumentSearchByPredicate);
    RUN_TEST(TestDocumentSearchByStatus);
    RUN_TEST(TestCalculateRelevance);
}

// --------- Окончание модульных тестов поисковой системы -----------

int main() {
    TestSearchServer();
    // Если вы видите эту строку, значит все тесты прошли успешно
    cout << "Search server testing finished"s << endl;
}