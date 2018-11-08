// NOTE: considering the simplicity of this program, I have opted to implement the entire thing in one file. 
//       This makes compiling with GCC trivial. 
#include <iostream>
#include <string>
#include <queue>
#include <unordered_map>
#include <algorithm>
#include "TextTable.h"

// This class represents a proxy of the std queue with page fault logic specific to out application
class fifo_table {
private:
    // Std deque used for simplicity. Integer also chosen as data type for simplicity. 
    // NOTE: Deque is chosen over queue for algorithms library compatibility. 
    std::deque<int> _pageQueue;
    // The storage capacity of our page table
    size_t _capacity;
    // The number of page faults encounteres since creation
    int _page_fault_count = 0;

public:
    // Trivial constructor. 
    fifo_table(size_t capacity) : _capacity(capacity) {}

    // Simply returns capacity. 
    int capacity() { return _capacity; }

    // Get a copy of the list of current pages
    std::vector<int> get_page_list() { 
        std::vector<int> pages;
        std::copy(_pageQueue.begin(), _pageQueue.end(), std::back_inserter(pages));
        return pages;

    }

    // Attempts to load a page size_to our queue.
    // argument - page: The page to be stored. 
    // return: Whether or not the system page faulted, paired with the page if replaced. 
    std::pair<bool, int> load_page(int page) {
        std::pair<bool, int> result = std::make_pair(false, -1);
        // If it is not in the queue. push something out. 
        if(std::find(_pageQueue.begin(), _pageQueue.end(), page) == _pageQueue.end()) {
            _page_fault_count++;
            result.first = true;
            _pageQueue.push_back(page);
            if(_pageQueue.size() > _capacity) {
                result.second = _pageQueue.front();
                _pageQueue.pop_front();
            }
        }
        return result;
    }

    // simply returns the number of page faults encountered
    int get_page_fault_count() {
        return _page_fault_count;
    }
};

// This class represents a proxy of the std queue with page fault logic specific to out application
class lru_table {
private:
    // unordered_map used for 0(1) searching and insertion. No need for ordering. 
    std::unordered_map<int, size_t> _pageMap;
    // The storage capacity of our page table
    size_t _capacity;
    // The number of page faults encounteres since creation
    int _page_fault_count = 0;

public:
    // Trivial constructor. 
    lru_table(size_t capacity) : _capacity(capacity) {}

    // Simply returns capacity. 
    int capacity() { return _capacity; }

    // Get a copy of the list of current pages
    std::vector<int> get_page_list() { 
        std::vector<int> pages;
        for(auto& kvp : _pageMap)
            pages.push_back(kvp.first);
        return pages;

    }

    // Attempts to load a page size_to our queue. 
    // argument - page: The page to be stored. 
    // return: Whether or not the system page faulted, paired with the page if replaced. 
    //         If page replaced is -1, we replaced an empty slot. 
    std::pair<bool, int> load_page(int page) {
        std::pair<bool, int> result = std::make_pair(false, -1);
        // If it is not in the queue, this will be a new entry.  
        if(_pageMap.find(page) == _pageMap.end()) {
            _page_fault_count++;
            result.first = true;
            _pageMap.insert(std::make_pair(page, 0));
        }
        // Going to set this to 0 regardless, so we do it outside the if. 
        _pageMap[page] = 0; // Set time to 0. For we have just used this. 

        // increment all time counters. NOTE: kvp => keyValuePair
        for(auto& keyValuePair : _pageMap) 
            keyValuePair.second += 1;

        // Eject maxima element if we're over capacity. 
        if(_pageMap.size() > _capacity) {
            // Compare by second value in pair. 
            auto compareFunction = [](auto& max, auto& kvp) { return max.second < kvp.second; };
            // Find element with greatest time. 
            auto maxElement = std::max_element(_pageMap.begin(), _pageMap.end(), compareFunction);
            result.second = maxElement->first; // set equal to page we're removing. 
            _pageMap.erase(maxElement);
        }
        return result;
    }

    // simply returns the number of page faults encountered
    int get_page_fault_count() {
        return _page_fault_count;
    }
};

int main() {
    std::array<int, 33> pageTestSet { 
        1, 1, 1, 1, 0, 3, 1, 1, 3, 5, 1, 8, 1, 3, 5, 13,
        15, 6, 1, 1, 3, 6, 7, 8, 9, 3, 1, 1, 4, 4, 4, 1, 2
    };
    std::array<size_t, 3> residentSetSizes {
        3, 5, 7
    };

    // Generic lambda assumes API of page table is the same. 
    // argument - page_table: The page table we will be using
    // argument - rss: The page table we will be using
    const auto run_trial = [&](auto& page_table) {
        if(page_table.capacity() == 3) {
            TextTable text_table;
            text_table.add("");
            text_table.add("New Page");
            text_table.add("Page Replaced");
            text_table.add("Current Page List");
            text_table.endOfRow();

            for(size_t i = 0; i < pageTestSet.size(); i++) {
                text_table.add("Trial " + std::to_string(i));

                // Add trial number. 
                int page = pageTestSet[i];
                text_table.add(std::to_string(page));

                // Attempt to load page. Record result
                auto result = page_table.load_page(page);
                if(result.first) {
                    // If we actually ejected a page, stringize it. Else just print 'empty' to signify empty slot
                    std::string result_string = (result.second != -1) ? std::to_string(result.second) : "empty";
                    text_table.add(result_string);
                }
                else{
                    text_table.add("none");
                }

                // Stringize current page list.  
                auto currentPageList = page_table.get_page_list();
                std::string pageListString = "";
                for(int page : currentPageList) 
                    pageListString += std::to_string(page) + ", ";
                text_table.add(pageListString);

                // Done
                text_table.endOfRow();
            }
            text_table.setAlignment(1, TextTable::Alignment::RIGHT);
            text_table.setAlignment(2, TextTable::Alignment::RIGHT);
            text_table.setAlignment(3, TextTable::Alignment::RIGHT);
            std::cout << text_table;
        }
        else {
            // Else we justrun normally. 
            for(int page : pageTestSet) {
                page_table.load_page(page);
            }
        }
    };

    // Setup Summary Table
    TextTable summary_table;
    summary_table.add("Resident Set Size");
    summary_table.add("# Faults using FIFO");
    summary_table.add("FIFO Page Fault Frequency");
    summary_table.add("# Faults using LRU");
    summary_table.add("LRU Page Fault Frequency");
    summary_table.endOfRow();

    for(size_t rss : residentSetSizes) {
        summary_table.add(std::to_string(rss));

        std::cout << "Data Set 1: FIFO, RSS = " << rss << std::endl;
        // Run fifo trial
        fifo_table fifo(rss);
        run_trial(fifo);
        summary_table.add(std::to_string(fifo.get_page_fault_count()));
        float fifo_fault_ratio = ((float)fifo.get_page_fault_count()) / ((float)pageTestSet.size());
        summary_table.add(std::to_string(fifo_fault_ratio));

        std::cout << "Data Set 1: LRU, RSS = " << rss << std::endl;
        // Run lru trial
        lru_table lru(rss);
        run_trial(lru);
        summary_table.add(std::to_string(lru.get_page_fault_count()));
        float lru_fault_ratio = ((float)lru.get_page_fault_count()) / ((float)pageTestSet.size());
        summary_table.add(std::to_string(lru_fault_ratio));

        summary_table.endOfRow();
    }

    std::cout << summary_table;
    return 0;
}