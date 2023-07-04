#include "remove_duplicates.h"
using namespace std::string_literals; 
 
void RemoveDuplicates(SearchServer& search_server) {
    std::set<int> id_remove;
    std::map<std::set<std::string>, int> unique_word_plus_id;
    for (const int document_id : search_server) {
        std::set<std::string> unique_words;
 
        for (auto [word, _] : search_server.GetWordFrequencies(document_id)) {
            unique_words.insert(word);
        }
 
        if (unique_word_plus_id.count(unique_words)) {
            id_remove.insert(document_id);
        } else {
            unique_word_plus_id.insert(std::pair{unique_words, document_id});
        }
    }
    for (auto id: id_remove) {
        std::cout<<"Found duplicate document id "<< id <<std::endl;
        search_server.RemoveDocument(id);
    }
}