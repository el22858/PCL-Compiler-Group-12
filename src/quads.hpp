#ifndef _QUADS_HPP_
#define _QUADS_HPP_

#include <iostream>
#include <string>
#include <vector>

struct quad {
    std::string op, x, y, z;

    quad(std::string opname, std::string op1, std::string op2, std::string op3) : op(opname), x(op1), y(op2), z(op3) {}

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
    out << q.op << ", " << q.x << ", " << q.y << ", " << q.z;
    return out;
}

inline std::ostream &operator<<(std::ostream &out, const std::vector<quad> &v) {
    int s = v.size();
    for (auto i=0; i<s; ++i) out << i+1 << ": " << v[i] << std::endl;
    return out;
}

int quadNEXTQUAD();

void quadGENQUAD(std::string op, std::string x, std::string y, std::string z);

int quadNEWTEMP();

std::vector<int> quadEMPTYLIST();

std::vector<int> quadMAKELIST(int x);

std::vector<int> quadMERGELISTS(std::vector<int> l1, std::vector<int> l2);

void quadBACKPATCH(std::vector<int> l, std::string newAd);

// std::string quadADDRESSOF(std::unique_ptr<Expr> e);

extern std::vector<quad> finalQuadList;

#endif