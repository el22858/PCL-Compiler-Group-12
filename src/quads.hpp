#ifndef _QUADS_HPP_
#define _QUADS_HPP_

#include <iostream>
#include <string>
#include <vector>


// Copied from https://stackoverflow.com/questions/4654636/how-to-determine-if-a-string-is-a-number-with-c
inline bool is_number(const std::string& s) {
    std::string::const_iterator it = s.begin();
    while (it != s.end() && std::isdigit(*it)) ++it;
    return !s.empty() && it == s.end();
}

inline bool is_real(const std::string &s) {
    if (s.empty()) return false;
    std::string::const_iterator it = s.begin();
    while (it != s.end() && (std::isdigit(*it))) ++it; // || (*it == '.')));
    if (*(it++) != '.') return false;
    while (it != s.end() && (std::isdigit(*it))) ++it; // || (*it == '.')));
    if (it == s.end()) return true;
    else if (*it != 'e') return false;
    it++;
    if ((*it == '+') || (*it == '-')) ++it;
    while ((it != s.end()) && (std::isdigit(*it))) ++it;
    return (it == s.end());
}

inline bool is_tmp(const std::string &s) {
    if (s.empty()) return false;
    std::string::const_iterator it = s.begin();
    if (*it != '$') return false;
    while ((++it != s.end()) && (std::isdigit(*it)));
    return it == s.end();
}


class quad {
    private:
        int tag;
        std::string op, x, y, z;
        int n_x, n_y, n_z;
        int x_off, y_off, z_off;
        int x_size, y_size, z_size;
        bool hasReal;

    public:
        quad(int t, std::string opname, std::string op1, std::string op2, std::string op3, bool hR = false, int nx = 0, int xoff = 0, int ny = 0, int yoff = 0, int nz = 0, int zoff = 0, int xs = 0, int ys = 0, int zs = 0) : tag(t), op(opname), x(op1), y(op2), z(op3), n_x(nx), n_y(ny), n_z(nz), x_off(xoff), y_off(yoff), z_off(zoff), x_size(xs), y_size(ys), z_size(zs), hasReal(hR) {}

        int getTag() const { return tag; }
        std::string getOpname() const { return op; }
        std::string getOp1() const { return x; }
        std::string getOp2() const { return y; }
        std::string getOp3() const { return z; }
        bool withReal() const { return hasReal; }


        void setOpname(std::string opname) { op = opname; }
        void setOp1(std::string op1) { x = op1; }
        void setOp2(std::string op2) { y = op2; }
        void setOp3(std::string op3) { z = op3; }
        void setReal(bool b) { hasReal = b; }

        int getXdepth() const { return n_x; }
        int getYdepth() const { return n_y; }
        int getZdepth() const { return n_z; }
        int getXoffset() const { return x_off; }
        int getYoffset() const { return y_off; }
        int getZoffset() const { return z_off; }
        int getXsize() const { return x_size; }
        int getYsize() const { return y_size; }
        int getZsize() const { return z_size; }

        void setXdepth(int xD) { n_x = xD; }
        void setYdepth(int yD) { n_y = yD; }
        void setZdepth(int zD) { n_z = zD; }
        void setXoffset(int xO) { x_off = xO; }
        void setYoffset(int yO) { y_off = yO; }
        void setZoffset(int zO) { z_off = zO; }
        void setXsize(int xS) { x_size = xS; }
        void setYsize(int yS) { y_size = yS; }
        void setZsize(int zS) { z_size = zS; }
};

inline std::ostream &operator<<(std::ostream &out, const quad &q) {
    out << q.getTag() << ": " << q.getOpname() << ", " << q.getOp1() << ", " << q.getOp2() << ", " << q.getOp3();
    // out << "quadGENQUAD(\"" << q.op << "\", \"" << q.x << "\", \"" << q.y << "\", \"" << q.z << "\");"; 
    return out;
}

inline std::ostream &operator<<(std::ostream &out, const std::vector<quad> &v) {
    for (const auto &q : v) out << q << std::endl;
    return out;
}

extern std::vector<quad> finalQuadList;
extern int quadNextTemp;

inline int quadNEXTQUAD() { return finalQuadList.size() + 1; }

inline void quadGENQUAD(std::string op, std::string x, std::string y, std::string z, bool b = false, int nx=0, int ny=0, int nz=0, int xoff=0, int yoff=0, int zoff=0, int xsize=0, int ysize=0, int zsize=0) { finalQuadList.push_back(quad(quadNEXTQUAD(), op, x, y, z, b, nx, xoff, ny, yoff, nz, zoff, xsize, ysize, zsize)); }

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

inline void quadBACKPATCH(std::vector<int> l, std::string newAd) { for (const auto &x : l) finalQuadList[x-1].setOp3(newAd); /* CAN'T BELIEVE I FORGOT THAT THIS IS ZERO-INDEXED DAMMITALL */ }

inline std::string quadADDRESSOF(std::string x) {
    std::string res = "";
    if ((x[0] == '[') &&  (x[x.length() - 1] == ']')) {
        for (unsigned long int i = 1; i < x.length() - 1; ++i) res += x[i];
    } else res = "{" + x + "}";
    return res;
}

inline void quadCOPY(quad &q1, const quad &q2) {
    q1.setOpname(q2.getOpname());
    q1.setOp1(q2.getOp1());
    q1.setOp2(q2.getOp2());
    q1.setOp3(q2.getOp3());
    q1.setReal(q2.withReal());
    
    q1.setXdepth(q2.getXdepth());
    q1.setYdepth(q2.getYdepth());
    q1.setZdepth(q2.getZdepth());
    
    q1.setXoffset(q2.getXoffset());
    q1.setYoffset(q2.getYoffset());
    q1.setZoffset(q2.getZoffset());

    q1.setXsize(q2.getXsize());    
    q1.setYsize(q2.getYsize());
    q1.setZsize(q2.getZsize());
}

inline bool quadIsJump(const quad &q) { return ((q.getOpname().compare("jump") == 0) || (q.getOpname().compare("jumpl") == 0)); }


#endif