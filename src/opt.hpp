#ifndef _OPT_HPP_
#define _OPT_HPP_

#include <cmath>
#include "quads.hpp"

extern bool hasChanged;

// Copied from https://stackoverflow.com/questions/4654636/how-to-determine-if-a-string-is-a-number-with-c
inline bool is_number(const std::string& s) {
    std::string::const_iterator it = s.begin();
    while (it != s.end() && std::isdigit(*it)) ++it;
    return !s.empty() && it == s.end();
}

inline bool is_real(const std::string &s) {
    if (s.empty()) return false;
    std::string::const_iterator it = s.begin();
    while (it != s.end() && (std::isdigit(*it) || (*it == '.')));
    return (it == s.end());
}

inline void algebraSimple() {
    for (auto &q : finalQuadList) {
        std::string op = q.getOpname();
        if ((op.compare("+") == 0) || (op.compare("-") == 0)) {
            if (is_number(q.getOp1())) {
                int o1 = std::stoi(q.getOp1());
                if (o1 == 0) {  // 0 + x = x and 0 - x = -x
                    if (op.compare("+") == 0) q.setOpname(":=");
                    q.setOp1(q.getOp2());
                    q.setOp2("-");
                    hasChanged = true;
                }
            }
            if (is_number(q.getOp2())) {  // x + 0 = x and x - 0 = x
                int o2 = std::stoi(q.getOp2());
                if (o2 == 0) {
                    q.setOpname(":=");
                    q.setOp2("-");
                    hasChanged = true;
                }
            }
        } else if (op.compare("*") == 0) {
            if (is_number(q.getOp1())) {
                int o1 = std::stoi(q.getOp1());
                if (o1 == 1) { // 1 * x = x
                    q.setOpname(":=");
                    q.setOp1(q.getOp2());
                    q.setOp2("-");
                    hasChanged = true;
                } else if ((o1 > 0) && !(o1 & (o1-1))) { // check if is power of 2
                    q.setOpname("<<");  // bit shifting is usually faster
                    q.setOp1(q.getOp2());
                    q.setOp2(std::to_string(int(log2(o1))));
                    hasChanged = true;
                }
            }
            if (is_number(q.getOp2())) {
                int o2 = std::stoi(q.getOp2());
                if (o2 == 1) {  // x * 1 = x
                    q.setOpname(":=");
                    q.setOp2("-");
                    hasChanged = true;
                } else if ((o2 > 0) && !(o2 & (o2-1))) { // check if is power of 2
                    q.setOpname("<<");  // bit shifting is usually faster
                    q.setOp2(std::to_string(int(log2(o2))));
                    hasChanged = true;
                }
            }
        } else if ((op.compare("div") == 0)) {
            if (is_number(q.getOp2())) {
                int o2 = std::stoi(q.getOp2());
                if (o2 == 1) {  // x div 1 = x
                    q.setOpname(":=");
                    q.setOp2("-");
                    hasChanged = true;
                } else if ((o2 > 0) && !(o2 & (o2-1))) { // check if is power of 2
                    q.setOpname(">>");  // bit shifting is definitely faster
                    q.setOp2(std::to_string(int(log2(o2))));
                    hasChanged = true;
                }
            }
        } else if (op.compare("mod") == 0) {
            if (is_number(q.getOp2())) {
                int o2 = std::stoi(q.getOp2());
                if (o2 == 1) {  // x mod 1 = 0 obviously
                    q.setOpname(":=");
                    q.setOp2("-");
                    q.setOp1("0");
                    hasChanged = true;
                } else if ((o2 > 0) && !(o2 & (o2-1))) { // check if is power of 2
                    q.setOpname("and");  // no need to do a division, "and" is a free opword 
                    q.setOp2(std::to_string(o2 - 1));
                    hasChanged = true;
                }
            }
        }
    }
}

inline void constantFolding() {
    for (auto &q : finalQuadList) {
        if ((is_number(q.getOp1())) && (is_number(q.getOp2()))) {
            std::string op = q.getOpname();
            int x = std::stoi(q.getOp1()), y = std::stoi(q.getOp2());
            q.setOpname(":=");
            q.setOp2("-");
            if (op.compare("+") == 0) q.setOp1(std::to_string(x + y));
            else if (op.compare("-") == 0) q.setOp1(std::to_string(x - y));
            else if (op.compare("*") == 0) q.setOp1(std::to_string(x * y));
            else if (op.compare("div") == 0) q.setOp1(std::to_string(x / y));
            else if (op.compare("mod") == 0) q.setOp1(std::to_string(x % y));
            else if (op.compare("<<") == 0) q.setOp1(std::to_string(x << y));
            else if (op.compare(">>") == 0) q.setOp1(std::to_string(x >> y));
            else if (op.compare("and") == 0) q.setOp1(std::to_string(x & y));

            else if (op.compare("=") == 0) {
                if (x != y) q.setOpname("cleanup");
                else q.setOpname("jump");
                q.setOp1("-");
                q.setOp2("-");
            } else if (op.compare("<>") == 0) {
                if (x == y) q.setOpname("cleanup");
                else q.setOpname("jump");
                q.setOp1("-");
                q.setOp2("-");
            } else if (op.compare("<") == 0) {
                if (x >= y) q.setOpname("cleanup");
                else q.setOpname("jump");
                q.setOp1("-");
                q.setOp2("-");
            } else if (op.compare("<=") == 0) {
                if (x > y) q.setOpname("cleanup");
                else q.setOpname("jump");
                q.setOp1("-");
                q.setOp2("-");
            } else if (op.compare(">") == 0) {
                if (x <= y) q.setOpname("cleanup");
                else q.setOpname("jump");
                q.setOp1("-");
                q.setOp2("-");
            } else if (op.compare(">=") == 0) {
                if (x < y) q.setOpname("cleanup");
                else q.setOpname("jump");
                q.setOp1("-");
                q.setOp2("-");
            }

            else std::cout << op << " has not yet been implemented" << std::endl;
            // std::cout << "dude" << std::endl;
            hasChanged = true;
        }
    }
}

inline void cleanup() {
    for (int i = 0; i < finalQuadList.size(); ++i) {
        if (finalQuadList[i].getOpname().compare("cleanup") == 0) {
            // std::cout << "CLEANUP TIME" << std::endl;
            finalQuadList.erase(finalQuadList.begin() + i);
            for (auto &q : finalQuadList) {
                if (is_number(q.getOp3())) {
                    int z = std::stoi(q.getOp3());
                    if (z > i + 1) q.setOp3(std::to_string(z-1));
                }
            }
            i--;
        }
    }
}

inline void optimize() {
    std::cout << "Optimizing." << std::endl;
    while (hasChanged){
        // std::cout << "Trying again." << std::endl;
        hasChanged = false;
        algebraSimple();
        constantFolding();
    }

    cleanup();
}

#endif