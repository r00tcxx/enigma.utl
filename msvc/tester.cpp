#include <iostream>
#include "retval.hpp"
#include "channel.hpp"
#include "trietree.hpp"
#include "memory.hpp"

using namespace enigma;

retval_i fun() {
    return { 3, "null" };
}
std::mutex m;
std::vector<int> arr, arr2;
void t1(channel<int>::consumer* c) {
    std::thread th([c] {
        int n = 0;

        while (n != 20) {
            int k = c->consume();
            {
                std::lock_guard<std::mutex> l(m);
                std::cout << "c1: " << k << std::endl;
                arr.push_back(k);
            }
            n++;
        }
        std::cout << "t1-ok" << std::endl;
        });
    th.detach();
}

void t2(channel<int>::consumer* c) {
    std::thread th([c] {
        int n = 0;
        while (n != 20) {
            int k = c->consume();
            {
                std::lock_guard<std::mutex> l(m);
                std::cout << "c2: " << k << std::endl;
                arr2.push_back(k);
            }
            n++;
        }
        std::cout << "t2-ok" << std::endl;
        });
    th.detach();
}

int main(int argc, char* argv[])
{
    //auto res = fun();
    //if (res.check([&](decltype(res)::const_type_ref i)->bool { return i == 3; }))
    //    std::cout << "ok" << std::endl;
    //std::cout << res.error();
    //std::cout << res.is_success();

    //int i = res + 1;
    //std::cout << "--" << i << std::endl;

    //channel<int> ch(2, 2);
    //auto sub = ch.new_consumer().val();
    //t1(sub);
    //t2(ch.new_consumer());

    //std::thread th([&] {
    //    auto pub1 = ch.new_producer().val();
    //    for (int n = 0; n < 10; n++) {
    //        pub1->publish(n);
    //    }
    //    });

    //std::thread th2([&] {
    //    auto pub2 = ch.new_producer().val();
    //    for (int n = 10; n < 20; n++) {
    //        pub2->publish(n);
    //    }
    //    });
    //th.join();
    //th2.join();


    //trietree<char>a;
    //a.insert("r1", 3);
    //a.insert("r2", 3);
    //a.insert("r3", 3);

    //a.insert("r1.1", 5);
    //a.insert("r1.2", 5);

    //a.insert("r2.1", 5);
    //a.insert("r2.2", 5);
    //a.insert("r2.3", 5);

    //a.insert("r3.1", 5);
    //a.insert("r3.2", 5);
    //a.insert("r3.3", 5);

    //auto res = a.query("r", 2);
    //trietree<char>::trie_layer l = res.m_val();
    //l.get_words([&](trietree<char>::trie_layer::calback_param p)->bool {
    //    std::cout << p << '\t';
    //    return true;
    //    });
    ////todo trielayer Õ¹¿ª 
    //a.clear();
    auto a = allocater::instance(nullptr);
    auto p = a->alloc(3).val();
    memcpy(p, "12", 3);
    std::cout << (char*)p.target();
    a->free(p);

    p = a->alloc(3).val();
    memcpy(p, "34", 3);
    std::cout << (char*)p.target();

    auto p2 = a->alloc(3).val();
    memcpy(p2, "56", 3);
    std::cout << (char*)p2.target();
    //union_ptr<std::vector<int>*, int*> a;

    auto p3 = a->alloc(sizeof("hello")).val();
    memcpy(p3, "hello", sizeof("hello"));
    std::cout << (char*)p3.target() << std::endl;


    struct test {
        test(const int x, const int y) : k(x), b(y) {
            std::cout << "test constructor\n";
        }
        ~test() {
            std::cout << "test distructor\n";
        }
        int k = 0, b = 0;
    };
    const auto size = sizeof(test);
    auto p4 = a->constructor<test>(2, 3).val();
    std::cout << p4.target()->k << " " << p4.target()->b << std::endl;
    a->destructor<test>(p4);
    a->debug_check_chain();
    allocater::release();

    getchar();
    return 0;
}