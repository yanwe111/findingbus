#include "query/station-matcher.h"
#include <algorithm>
#include <cctype>

/// Simple Vietnamese diacritics removal (UTF-8 aware, covers common chars)
static string strip_vietnamese_diacritics(const string& input) {
    // Map common Vietnamese UTF-8 sequences to ASCII equivalents
    // This is a simplified approach — handles most common characters
    string result;
    result.reserve(input.size());

    for (size_t i = 0; i < input.size(); i++) {
        unsigned char c = static_cast<unsigned char>(input[i]);
        if (c < 0x80) {
            result += static_cast<char>(tolower(c));
        } else if (c == 0xC3 && i + 1 < input.size()) {
            // Latin Extended: à á â ã è é ê ì í ò ó ô õ ù ú ý
            unsigned char next = static_cast<unsigned char>(input[i + 1]);
            if (next >= 0xA0 && next <= 0xA5) result += 'a';
            else if (next >= 0xA8 && next <= 0xAB) result += 'e';
            else if (next >= 0xAC && next <= 0xAF) result += 'i';
            else if (next >= 0xB2 && next <= 0xB6) result += 'o';
            else if (next >= 0xB9 && next <= 0xBC) result += 'u';
            else if (next == 0xBD) result += 'y';
            else { result += c; result += input[i + 1]; }
            i++;
        } else if (c == 0xC4 || c == 0xC5) {
            // Extended Latin for đ, ơ, ư etc.
            if (i + 1 < input.size()) {
                unsigned char next = static_cast<unsigned char>(input[i + 1]);
                if (c == 0xC4 && (next == 0x90 || next == 0x91)) result += 'd';
                else { result += c; result += input[i + 1]; }
                i++;
            }
        } else if (c == 0xE1 && i + 2 < input.size()) {
            // Vietnamese extended: ắ ằ ẳ ẵ ặ ấ ầ ẩ ẫ ậ ế ề ể ễ ệ ố ồ ổ ỗ ộ ớ ờ ở ỡ ợ ứ ừ ử ữ ự ỳ ỵ ỷ ỹ
            unsigned char b2 = static_cast<unsigned char>(input[i + 1]);
            unsigned char b3 = static_cast<unsigned char>(input[i + 2]);
            if (b2 == 0xBA || b2 == 0xBB) {
                // Most Vietnamese vowels with marks
                if (b3 >= 0xA0 && b3 <= 0xBF) {
                    if (b2 == 0xBA) {
                        if (b3 <= 0xAF) result += 'a';
                        else if (b3 <= 0xBF) result += 'e';
                    } else {
                        if (b3 <= 0x8F) result += 'i';
                        else if (b3 <= 0x9F) result += 'o';
                        else if (b3 <= 0xAF) result += 'o';
                        else result += 'u';
                    }
                } else {
                    result += c; result += input[i+1]; result += input[i+2];
                }
            } else {
                result += c; result += input[i+1]; result += input[i+2];
            }
            i += 2;
        } else {
            result += c;
        }
    }
    return result;
}

string StationMatcher::normalize(const string& s) {
    string stripped = strip_vietnamese_diacritics(s);
    // Also normalize common abbreviations
    string result;
    for (char c : stripped) {
        if (isalnum(static_cast<unsigned char>(c)) || c == ' ') {
            result += c;
        }
    }
    return result;
}

int StationMatcher::edit_distance(const string& a, const string& b) {
    // Calculate the number of operations to change a into b 
    int m = static_cast<int>(a.size());
    int n = static_cast<int>(b.size());
    vector<vector<int>> dp(m + 1, vector<int>(n + 1, 0));

    for (int i = 0; i <= m; i++) dp[i][0] = i;
    for (int j = 0; j <= n; j++) dp[0][j] = j;

    for (int i = 1; i <= m; i++) {
        for (int j = 1; j <= n; j++) {
            if (a[i-1] == b[j-1]) dp[i][j] = dp[i-1][j-1];
            else dp[i][j] = 1 + min({dp[i-1][j], dp[i][j-1], dp[i-1][j-1]});
        }
    }
    return dp[m][n];
}

void StationMatcher::load_stops(const unordered_map<string, Stop>& stops) {
    entries_.clear();
    for (auto& [id, s] : stops) {
        if (s.name == "Waypoint" || s.name == "Unnamed Stop") continue;
        entries_.push_back({id, s.name, normalize(s.name)});
    }
}

vector<MatchResult> StationMatcher::match(const string& query, int top_n) const {
    string norm_query = normalize(query);

    vector<MatchResult> results;
    for (auto& entry : entries_) {
        double score;
        // Check substring match first (best score)
        if (entry.normalized.find(norm_query) != string::npos) {
            score = 0.0; // exact substring match
        } else if (norm_query.find(entry.normalized) != string::npos) {
            score = 0.5;
        } else {
            score = static_cast<double>(edit_distance(norm_query, entry.normalized));
        }
        results.push_back({entry.id, entry.name, score});
    }

    sort(results.begin(), results.end(),
              [](const MatchResult& a, const MatchResult& b) {
                  return a.score < b.score;
              });

    if (static_cast<int>(results.size()) > top_n) results.resize(top_n);
    return results;
}
