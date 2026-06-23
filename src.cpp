#include <iostream>
#include <fstream>
#include <string>
#include <vector>
#include <queue>

using namespace std;

int ram_acc = 0;
int l1_h = 0;
int l2_h = 0;
int l3_h = 0;
int tot_cyc = 0;
int cyc = 1;
int comp_task = 0;

// ==========================================
// MODULE 1: CACHE SYSTEM
// ==========================================
struct cache_data {
    int cap;
    vector<string> arr;
};

class CacheSystem {
public:
    cache_data L1;
    cache_data L2;
    cache_data L3;

    CacheSystem() {
        L1.cap = 32;
        L2.cap = 128;
        L3.cap = 512;
    }

    bool check_cache(cache_data c, string b) {
        for (int i = 0; i < c.arr.size(); i = i + 1) {
            if (c.arr[i] == b) {
                return true;
            }
        }
        return false;
    }

    string put_in_cache(cache_data &c, string b) {
        bool found = false;
        for (int i = 0; i < c.arr.size(); i = i + 1) {
            if (c.arr[i] == b) {
                found = true;
            }
        }
        if (found == true) {
            return "";
        }
        string evic = "";
        if (c.arr.size() >= c.cap) {
            evic = c.arr[0];
            c.arr.erase(c.arr.begin());
        }
        c.arr.push_back(b);
        return evic;
    }

    int get_mem(string b) {
        if (check_cache(L1, b) == true) {
            l1_h = l1_h + 1;
            cout << "L1 -> HIT (4 cycles)\n";
            return 4;
        }
        cout << "L1 -> MISS\n";

        if (check_cache(L2, b) == true) {
            l2_h = l2_h + 1;
            cout << "L2 -> HIT (12 cycles)\n";
            string e = put_in_cache(L1, b);
            if (e != "") cout << "Evicted from L1: " << e << "\n";
            return 12;
        }
        cout << "L2 -> MISS\n";

        if (check_cache(L3, b) == true) {
            l3_h = l3_h + 1;
            cout << "L3 -> HIT (40 cycles)\n";
            string e = put_in_cache(L1, b);
            if (e != "") cout << "Evicted from L1: " << e << "\n";
            return 40;
        }
        cout << "L3 -> MISS\n";

        cout << "Fetching from RAM (200 cycles)\n";
        ram_acc = ram_acc + 1;
        string e = put_in_cache(L1, b);
        if (e != "") cout << "Evicted from L1: " << e << "\n";
        return 200;
    }

    void show_c(cache_data c, string n) {
        cout << n << ": [";
        for (int i = 0; i < c.arr.size(); i = i + 1) {
            cout << c.arr[i];
            if (i != c.arr.size() - 1) {
                cout << ", ";
            }
        }
        cout << "]\n";
    }

    void show_all() {
        show_c(L1, "L1");
        show_c(L2, "L2");
        show_c(L3, "L3");
    }
};

// ==========================================
// MODULE 2: SCHEDULER SYSTEM
// ==========================================
struct task_data {
    string name;
    int b;
    int rem;
    vector<string> blocks;
    int index;
};

class Scheduler {
public:
    queue<task_data> q;
    int qtm;

    Scheduler() {
        qtm = 3; // Round Robin Quantum
    }

    // Load tasks from file
    bool load_tasks(string filename) {
        ifstream fin;
        fin.open(filename);

        if (fin.is_open() == false) {
            cout << "Could not open " << filename << "\n";
            return false;
        }

        string temp;
        while (getline(fin, temp)) {
            if (temp == "") continue;

            task_data t;
            t.index = 0;
            string curr_word = "";
            vector<string> words;

            for (int i = 0; i < temp.length(); i = i + 1) {
                if (temp[i] == ' ') {
                    if (curr_word != "") words.push_back(curr_word);
                    curr_word = "";
                } else {
                    curr_word = curr_word + temp[i];
                }
            }
            if (curr_word != "") words.push_back(curr_word);

            t.name = words[1];

            // Manual string to int conversion
            int val = 0;
            for (int i = 0; i < words[3].length(); i = i + 1) {
                val = val * 10 + (words[3][i] - '0');
            }

            t.b = val;
            t.rem = val;
            for (int i = 5; i < words.size(); i = i + 1) {
                t.blocks.push_back(words[i]);
            }
            q.push(t);
        }
        fin.close();
        return true;
    }

    void run(CacheSystem &cache) {
        while (q.empty() == false) {
            task_data curr = q.front();
            q.pop();

            int time_to_run = qtm;
            if (curr.rem < qtm) {
                time_to_run = curr.rem;
            }

            for (int k = 0; k < time_to_run; k = k + 1) {
                int mod_index = curr.index % curr.blocks.size();
                string blk = curr.blocks[mod_index];

                cout << "\n========================================\n";
                cout << "Cycle " << cyc << "\n";
                cout << "Running Task: " << curr.name << "\n";
                cout << "Requesting Block: " << blk << "\n\n";

                // Interact with the cache module
                int ans = cache.get_mem(blk);
                tot_cyc = tot_cyc + ans;

                cout << "\n";
                cache.show_all();

                curr.index = curr.index + 1;
                curr.rem = curr.rem - 1;
                cyc = cyc + 1;

                if (curr.rem == 0) break;
            }

            if (curr.rem > 0) {
                q.push(curr);
            } else {
                comp_task = comp_task + 1;
                cout << "\nTask " << curr.name << " completed.\n";
            }
        }
    }
};

// ==========================================
// MAIN FUNCTION
// ==========================================
int main() {
    Scheduler my_scheduler;
    CacheSystem my_cache;

    if (my_scheduler.load_tasks("input.txt") == false) {
        return 1;
    }

    my_scheduler.run(my_cache);

    cout << "\n========================================\n";
    cout << "FINAL RESULTS\n";
    cout << "========================================\n";
    cout << "Scheduler: Round Robin\n";
    cout << "Quantum: " << my_scheduler.qtm << "\n";
    cout << "Tasks Completed: " << comp_task << "\n";
    cout << "Total Cycles: " << tot_cyc << "\n";
    cout << "L1 Hits: " << l1_h << "\n";
    cout << "L2 Hits: " << l2_h << "\n";
    cout << "L3 Hits: " << l3_h << "\n";
    cout << "RAM Accesses: " << ram_acc << "\n";
    cout << "\nFinal Cache State\n";
    my_cache.show_all();

    return 0;
}