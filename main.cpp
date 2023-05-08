#include <iostream>
#include "table.h"

using namespace std;

int main() {

    Row r1 = Row(-11, "aaa", "ddd");
    Row r2 = Row(100, "bbb", "adsd");
    Row r3 = Row(20, "ccc", "###");
    Row r4 = Row(-2200, "asd", "dsaasd");
    Row r5 = Row(2, "dsad", "asdsda");
    Row r6 = Row(-25, "asddsad", "asdsad");


    Table t;
    Table t2;
    t.emplace_back(r1);
    t2.emplace_back(r2);
    cout << t << endl;
    cout << t2 << endl;

    t.emplace_back(r3);
    t2.emplace_back(r4);
    t.emplace_back(r5);
    t2.emplace_back(r6);
    t.emplace_back("Col 1", "Cooolllll 22222");
    t2.emplace_back("Col 1", "Cooolllll 22222");
    t.emplace_back("Col 1", "Cooolllll 22222");



    auto res_1 = t.get_sorted_range(1, 6, COLUMN_2);
    auto res_2 = t.get_sorted_range(5, 50, COLUMN_1);

    cout << Table(res_1) << endl;

    cout << endl;

    cout << Table(res_2) << endl;

    cout << endl;
    cout << t << endl;

    cout << t[1] << endl;
    cout << t[100] << endl;

    return 0;
}

