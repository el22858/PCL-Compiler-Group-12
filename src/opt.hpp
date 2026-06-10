#ifndef _OPT_HPP_
#define _OPT_HPP_

#include <cmath>
#include <algorithm>
#include "quads.hpp"

extern bool hasChanged;
// extern std::vector<std::pair<std::string, int>> tmpLine;
extern std::vector<int> labels, jumps;


// inline void findTmps() {
//     for (const auto &q : finalQuadList) {
//         if (is_tmp(q.getOp3())) tmpLine.push_back(std::make_pair(q.getOp3(), q.getTag()));
//     }
// }

inline void makeBlocks() { // FIXME: is missing out on quite a few jumps, possibly other stuff too
    labels.push_back(0);
    jumps.push_back(finalQuadList.size() - 1);

    for (const auto &q : finalQuadList) {
        std::string op = q.getOpname();
        if (op.compare("label") == 0) labels.push_back(q.getTag()-1);
        else if (op.compare("unit") == 0) labels.push_back(q.getTag()-1); 

        else if (op.compare("jump") == 0) {
            jumps.push_back(q.getTag()-1);
            labels.push_back(std::stoi(q.getOp3())-1);
        } else if (op.compare("ifb") == 0) {
            jumps.push_back(q.getTag()-1);
            labels.push_back(std::stoi(q.getOp3())-1);
        } else if (op.compare("=") == 0) {
            jumps.push_back(q.getTag()-1);
            labels.push_back(std::stoi(q.getOp3())-1);
        } else if (op.compare("<>") == 0) {
            jumps.push_back(q.getTag()-1);
            labels.push_back(std::stoi(q.getOp3())-1);
        } else if (op.compare("<") == 0) {
            jumps.push_back(q.getTag()-1);
            labels.push_back(std::stoi(q.getOp3())-1);
        } else if (op.compare("<=") == 0) {
            jumps.push_back(q.getTag()-1);
            labels.push_back(std::stoi(q.getOp3())-1);
        } else if (op.compare(">") == 0) {
            jumps.push_back(q.getTag()-1);
            labels.push_back(std::stoi(q.getOp3())-1);
        } else if (op.compare(">=") == 0) {
            jumps.push_back(q.getTag()-1);
            labels.push_back(std::stoi(q.getOp3())-1);
        }
    }

    std::sort(labels.begin(), labels.end());
    auto it = std::unique(labels.begin(), labels.end());
    // if (it < labels.end()) std::cout << "WUT" << std::endl;

    labels.erase(it, labels.end());
    // for (const auto &l : labels) std::cout << l << std::endl;
    // while (it != labels.end()) {
        //     std::cout << "dude" << std::endl;
        //     labels.erase(it);
        //     // ++it;
    // }
        
    std::sort(jumps.begin(), jumps.end());
}

inline void algebraSimple(int start = 0, int end = finalQuadList.size()-1) {
    for (int i = start; i <= end; i++) {
        auto &q = finalQuadList[i];
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

inline void constantFolding(int start = 0, int end = finalQuadList.size()-1) {
    for (int i = start; i <= end; ++i) {
        auto &q = finalQuadList[i];
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
                // std::cout << "DAFUQ " << x << " " << y << std::endl; 
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

inline void singleAss(int start = 0, int end = finalQuadList.size()-1) {
    for (int i = start; i < end; ++i) {
        auto &q = finalQuadList[i];
        std::string op = q.getOpname(), z = q.getOp3();
        if ((op.compare("+") == 0) || (op.compare("-") == 0) || (op.compare("*") == 0) || (op.compare("div") == 0) || (op.compare("mod") == 0) || (op.compare("/") == 0) || (op.compare("<<") == 0) || (op.compare(">>") == 0) || (op.compare("and") == 0) || (op.compare(":=") == 0)) {
            for (int j = i+1; j <= end; ++j) {
                if (z.compare(finalQuadList[j].getOp3()) == 0) {
                    std::string newZ = "$" + std::to_string(quadNEWTEMP());
                    q.setOp3(newZ);
                    for (int k = i+1; k <= j; ++k) {
                        if (z.compare(finalQuadList[k].getOp1()) == 0) finalQuadList[k].setOp1(newZ);
                        if (z.compare(finalQuadList[k].getOp2()) == 0) finalQuadList[k].setOp2(newZ);
                    }
                    break;
                }
            }
        }
    }
}

inline void commonSubElim(int start = 0, int end = finalQuadList.size()-1) {
    for (int i=start; i <= end; ++i) {
        std::string op = finalQuadList[i].getOpname(), x = finalQuadList[i].getOp1(), y = finalQuadList[i].getOp2(), z = finalQuadList[i].getOp3();

        for (int j = i+1; j <= end; ++j) {
            auto &q = finalQuadList[j];
            if ((op.compare(q.getOpname()) == 0) && (x.compare(q.getOp1()) == 0) && (y.compare(q.getOp2()) == 0) && (z.compare(q.getOp3()) != 0)) {
                q.setOpname(":=");
                q.setOp1(z);
                q.setOp2("-");
                hasChanged = true;
            }
        }
    }
}

inline void localCopyProp(int start = 0, int end = finalQuadList.size()-1) {
    for (int i = start; i<= end; ++i) {
        auto &q = finalQuadList[i];
        if (q.getOpname().compare(":=") == 0) {
            std::string x = q.getOp1(), z = q.getOp3();
            for (int j = i+1; j <= end; ++j) {
                auto &p = finalQuadList[j];
                if (p.getOp1().compare(z) == 0) p.setOp1(x);
                if (p.getOp2().compare(z) == 0) p.setOp2(x);
            }
        }
    }
}

inline void revProp(int start = 0, int end = finalQuadList.size()-1) {
    for (int i = start; i <= end; ++i) {
        auto &q = finalQuadList[i];
        if (q.getOpname().compare(":=") == 0) {
            std::string z = q.getOp3();
            if (is_tmp(q.getOp1())) {
                std::string temp = q.getOp1();
                for (int j = i-1; j >= start; --j) {
                    auto &v = finalQuadList[j];
                    if (v.getOp3().compare(temp) == 0) {
                        v.setOp3(z);
                        q.setOpname("cleanup");
                        break;
                    }
                }
            }
        }
    }
}

inline void localOpts(int start = 0, int end = finalQuadList.size()-1) {
    std::cout << start << " " << end << std::endl;

    singleAss(start, end);
    while (hasChanged){
        // std::cout << "Trying again." << std::endl;
        hasChanged = false;
        algebraSimple(start, end);
        constantFolding(start, end);
        commonSubElim(start, end);
        localCopyProp(start, end);
    }
    // revProp(start, end);
}

inline void cleanup() {
    for (int i = 0; i < finalQuadList.size(); ++i) {
        if (finalQuadList[i].getOpname().compare("cleanup") == 0) {
            // std::cout << "CLEANUP TIME" << std::endl;
            finalQuadList.erase(finalQuadList.begin() + i);
            // for (auto &q : finalQuadList) {
            //     if (is_number(q.getOp3())) {
            //         int z = std::stoi(q.getOp3());
            //         if (z > i + 1) q.setOp3(std::to_string(z-1));
            //     }
            // }
            i--;
        }
    }
}

inline void globalOpts() { //FIXME
    /* globalCopyProp(); globalAssassin(); */
    cleanup();
}


inline void peepOpts() {
    for (int i=0; i < finalQuadList.size(); ++i) {
        auto &q = finalQuadList[i];
        if (q.getOpname().compare("jump") == 0) {
            int z = std::stoi(q.getOp3());
            if (finalQuadList[i+1].getTag() == z) q.setOpname("cleanup");
            else if (quadIsJump(finalQuadList[z-1])) quadCOPY(q, finalQuadList[z-1]);
        } else if (q.getOpname().compare("jumpl") == 0) {
            std::string label = q.getOp3();
            for (int j = 0; j < finalQuadList.size(); ++j) {
                if ((finalQuadList[j].getOpname().compare("label") == 0) && (finalQuadList[j].getOp3().compare(label) == 0)) {
                    if (j == i+1) q.setOpname("cleanup");
                    else if (quadIsJump(finalQuadList[j+1])) quadCOPY(q, finalQuadList[j+1]);
                    break;
                }
            }
        }
    }

    cleanup();
}




inline void optimize() {
    // std::cout << "Optimizing." << std::endl;
    makeBlocks();
    int i = 0, j = 0;
    
    while ((i < labels.size()) || (j < jumps.size())) {
        hasChanged = true;
        if (j == jumps.size()) {
            if (i < labels.size() - 1) localOpts(labels[i], labels[++i]);
            else localOpts(labels[i++], finalQuadList.size() - 1);
        } else if (i == labels.size()) {
            if (j == jumps.size() - 1) break;
            localOpts(jumps[j], jumps[++j]);
        } else if (labels[i] > jumps[j]) localOpts(jumps[j], jumps[++j]);
        else if ((i < labels.size() - 2) && (labels[i+1] < jumps[j])) localOpts(labels[i], labels[++i]-1);
        else localOpts(labels[i++], jumps[j++]);
    }
    // findTmps();

    cleanup();
    // globalOpts(); // currently ddoes nothing, might not implement
    peepOpts();


}

#endif