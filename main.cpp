#include <iostream>
#include <fstream>
#include <sstream>
#include <iomanip>
#include <unordered_map>
#include <map>
#include <vector>

using namespace std;

struct Order {
    char side;
    double price;
    int size;
};

// Format price to string with 2 decimal places or blank
string format_price(double price) {
    if (price == 0.0) return "";
    ostringstream out;
    out << fixed << setprecision(2) << price;
    return out.str();
}

int main(int argc, char* argv[]) {
    if (argc < 2) {
        cerr << "Usage: ./mbp <mbo_input.csv>\n";
        return 1;
    }

    ifstream infile(argv[1]);
    if (!infile.is_open()) {
        cerr << "Error opening input file.\n";
        return 1;
    }

    ofstream outfile("mbp_output.csv");

    unordered_map<string, Order> order_book;
    map<double, pair<int, int>, greater<>> bids;  // Descending
    map<double, pair<int, int>> asks;             // Ascending

    string line;
    getline(infile, line); // skip header

    // Output exact MBP header
    outfile << ",ts_recv,ts_event,rtype,publisher_id,instrument_id,action,side,depth,price,size,flags,ts_in_delta,sequence";
    for (int i = 0; i < 10; ++i) {
        outfile << ",bid_px_" << setfill('0') << setw(2) << i
                << ",bid_sz_" << setfill('0') << i
                << ",bid_ct_" << setfill('0') << i
                << ",ask_px_" << setfill('0') << i
                << ",ask_sz_" << setfill('0') << i
                << ",ask_ct_" << setfill('0') << i;
    }
    outfile << ",symbol,order_id\n";

    int row_id = 0;
    while (getline(infile, line)) {
        vector<string> row;
        string token;
        istringstream ss(line);
        while (getline(ss, token, ',')) row.push_back(token);
        if (row.size() < 15) continue;

        // Read required fields
        string ts_recv = row[0];
        string ts_event = row[1];
        string rtype = "10";  // Fixed as per format
        string publisher_id = row[3];
        string instrument_id = row[4];
        string action = row[5];
        string side = row[6];
        string price_str = row[7];
        string size_str = row[8];
        string depth = "0";
        string flags = row[11];
        string ts_in_delta = row[12];
        string sequence = row[13];
        string symbol = row[14];
        string order_id = row[10];

        double price = price_str.empty() ? 0.0 : stod(price_str);
        int size = size_str.empty() ? 0 : stoi(size_str);

        // Update order book
        if (action == "R") {
            order_book.clear();
        } else if (action == "A") {
            order_book[order_id] = { side[0], price, size };
        } else if (action == "C") {
            order_book.erase(order_id);
        }

        // Rebuild bid/ask maps
        bids.clear();
        asks.clear();
        for (const auto& [oid, ord] : order_book) {
            if (ord.side == 'B') {
                auto& level = bids[ord.price];
                level.first += ord.size;
                level.second += 1;
            } else {
                auto& level = asks[ord.price];
                level.first += ord.size;
                level.second += 1;
            }
        }

        // Output row
        outfile << row_id++ << "," << ts_recv << "," << ts_event << "," << rtype << "," << publisher_id << ","
                << instrument_id << "," << action << "," << side << "," << depth << ","
                << price_str << "," << size_str << "," << flags << "," << ts_in_delta << "," << sequence;

        // Top 10 levels of MBP
        auto bid_it = bids.begin();
        auto ask_it = asks.begin();
        for (int i = 0; i < 10; ++i) {
            if (bid_it != bids.end()) {
                outfile << "," << format_price(bid_it->first)
                        << "," << bid_it->second.first
                        << "," << bid_it->second.second;
                ++bid_it;
            } else {
                outfile << ",,,";
            }

            if (ask_it != asks.end()) {
                outfile << "," << format_price(ask_it->first)
                        << "," << ask_it->second.first
                        << "," << ask_it->second.second;
                ++ask_it;
            } else {
                outfile << ",,,";
            }
        }

        outfile << "," << symbol << "," << order_id << "\n";
    }

    infile.close();
    outfile.close();
    cout << "✅ Strict MBP output written to 'mbp_output.csv'\n";
    return 0;
}
