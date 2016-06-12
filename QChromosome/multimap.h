#ifndef MULTIMAP_H
#define MULTIMAP_H


// wierzchołek pełnego drzewa binarnego
class Node
{
public:
    Node(unsigned f, double v);
    Node(unsigned f, double v, Node* l, Node* r);
    ~Node();

    double minimum(unsigned lb, unsigned rb, double lm, double rm) const;
    double maximum(unsigned lb, unsigned rb, double lm, double rm) const;

private:
    // lewe i prawe poddrzewo
    Node *left, *right;

    // ramka i wartość funkcji
    unsigned frame;
    double value;

    // końce reprezentowanego przedziału ramek
    unsigned lbound, rbound;

    // maksymalna i minimalna wartość w poddrzewie
    double minv, maxv;

    // wartość najwcześniejszego wierzchołka w poddrzewie
    double leftm;

    // głębokość poddrzewa
    unsigned size;

friend class Nodes;
};

#include <QList>

class Nodes
{
public:
    Nodes();
    ~Nodes();

    void insert(unsigned frame, unsigned value);

    double minimum(unsigned lbound, unsigned rbound) const;
    double maximum(unsigned lbound, unsigned rbound) const;

    bool visible;

private:
    QList<Node*> nodes;
};

#include <QMap>

class Tree
{
public:
    Tree();

    void insert(std::string fname, unsigned frame, unsigned value);

    double minimum(unsigned lbound, unsigned rbound) const;
    double maximum(unsigned lbound, unsigned rbound) const;

    void setVisible(const std::string& fname, bool visible);

    bool empty() const;

    void clear();

private:
    QMap<std::string, Nodes> roots;
};

#endif // MULTIMAP_H
