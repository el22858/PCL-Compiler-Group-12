#ifndef _QUADS_HPP_
#define _QUADS_HPP_

#include <iostream>
#include <string>
#include <vector>

struct quad {
    int tag;
    std::string op, x, y, z;

    quad(int t, std::string opname, std::string op1, std::string op2, std::string op3) : tag(t), op(opname), x(op1), y(op2), z(op3) {}

    int getTag() { return tag; }
    std::string getOpname() { return op; }
    std::string getOp1() { return x; }
    std::string getOp2() { return y; }
    std::string getOp3() { return z; }

    void setOpname(std::string opname) { op = opname; }
    void setOp1(std::string op1) { x = op1; }
    void setOp2(std::string op2) { y = op2; }
    void setOp3(std::string op3) { z = op3; }
};

inline std::ostream &operator<<(std::ostream &out, const quad &q) {
    out << q.tag << ": " << q.op << ", " << q.x << ", " << q.y << ", " << q.z;
    return out;
}

inline std::ostream &operator<<(std::ostream &out, const std::vector<quad> &v) {
    for (const auto q : v) out << q << std::endl;
    return out;
}

extern std::vector<quad> finalQuadList;
extern int quadNextTemp;

inline int quadNEXTQUAD() { return finalQuadList.size() + 1; }

inline void quadGENQUAD(std::string op, std::string x, std::string y, std::string z) { finalQuadList.push_back(quad(quadNEXTQUAD(), op, x, y, z)); }

inline int quadNEWTEMP() { return quadNextTemp++; }

inline std::vector<int> quadEMPTYLIST() {
    std::vector<int> list;
	return list;
}

inline std::vector<int> quadMAKELIST(int x) {
    std::vector<int> list;
	list.push_back(x);
	return list;
}

inline std::vector<int> quadMERGELISTS(std::vector<int> l1, std::vector<int> l2) {
	std::vector<int> l;

	if (l1.size() >= l2.size()) {
		l = l1;
		l.insert(l.end(), l2.begin(), l2.end());
	} else {
		l = l2;
		l.insert(l.begin(), l1.begin(), l1.end());
	}
	return l;
}

inline void quadBACKPATCH(std::vector<int> l, std::string newAd) { for (const auto &x : l) finalQuadList[x].setOp3(newAd); }

// std::string quadADDRESSOF(std::unique_ptr<Expr> e);


#endif