#include <iostream>
#include <fstream>
#include <string>
#include <sstream>
#include <vector>

using namespace std;

const unsigned MEMORY_SIZE = 1000;  // 初始化申请空间大小

struct map {
    unsigned m_size;
    char *m_addr;
    map *next, *prior;

    explicit map(unsigned m_size, char *m_addr) : m_size(m_size), m_addr(m_addr), next(nullptr),
                                                  prior(nullptr) {}  // 构造函数
};

map *head;  // 用于存储空闲区链表(指向当前节点)
char *p;  // 初始化申请空间的首地址

//  初始化函数
void init() {
    p = (char *) malloc(sizeof(MEMORY_SIZE));  // 初始化申请空间
    auto map_new = new map(MEMORY_SIZE, p);  // 初始化空闲区链表
    head = map_new;
    head->next = head;
    head->prior = head;
}

// 打印空闲存储区情况
void print() {
    cout << "======Print free area list======" << endl;
    if (head == nullptr) {  // 空闲区使用完的情况
        cout << "==============End===============" << endl << endl << endl;
        return;
    }
    // 从当前节点开始，打印空闲区链表的所有节点(地址偏移量+空闲区大小)
    auto ptr = head;
    cout << "address: " << ptr->m_addr - p << "   size: " << ptr->m_size << endl;
    ptr = ptr->next;
    while (ptr != head) {
        cout << "address: " << ptr->m_addr - p << "   size: " << ptr->m_size << endl;
        ptr = ptr->next;
    }
    cout << "==============End===============" << endl << endl << endl;
}

// 内存分配的函数
// 如果内存分配失败，则返回空指针，否则返回分配内存的首地址
char *lmalloc(unsigned size) {

    // 没有任何空闲区的情况
    if (head == nullptr || size == 0) return nullptr;

    auto ptr = head;

    // 从当前节点开始，遍历链表，直到找到可以分配的空闲内存
    while (true) {

        // 空闲区的大小与请求的分配大小相同
        if (ptr->m_size == size) {
            // 刚好只有一个空闲区的情况，置空闲区链表当前节点为空
            if (ptr->next == ptr) {
                auto res = ptr->m_addr;
                head = nullptr;
                delete ptr;
                return res;
            }
            // 分配完后还有剩余的空闲区，从空闲区链表中删除当前节点，head指向下一个空闲区
            auto pre = ptr->prior;
            auto next = ptr->next;
            auto res = ptr->m_addr;
            pre->next = next;
            next->prior = pre;
            head = next;
            delete ptr;
            return res;
        }

        // 空闲区的大小大于请求分配的大小
        // 重新设定当前空闲区的大小与首地址
        if (ptr->m_size > size) {
            auto res = ptr->m_addr;
            ptr->m_addr = ptr->m_addr + size;
            ptr->m_size = ptr->m_size - size;
            head = ptr->next;
            return res;
        }

        // 当前空闲区的大小无法满足分配需求，移动到下一个空闲区节点
        ptr = ptr->next;

        // 循环一圈也无法找到足够大小的空闲区，返回空指针
        if (ptr == head) return nullptr;
    }
}

// 内存释放的函数
// 从address开始，释放大小为size的内存
void lfree(unsigned size, char *address) {

    // 对于size=0以及address为空的情况，直接返回
    if (size == 0 || address == nullptr) return;

    // 如果当前已经没有空闲区了，则释放的内存成为空闲区的唯一一个节点
    if (head == nullptr) {
        // 如果address在申请空间外，则直接返回
        if (address < p || address + size >= p + MEMORY_SIZE) {
            cout << "The area that needs to be released is outside the memory area!" << endl;
            return;
        }
        // 创建新的空闲区节点
        auto map_new = new map(size, address);
        head = map_new;
        head->next = head;
        head->prior = head;
        return;
    }

    // 寻找从申请空间开始的第一个空闲区链表节点
    auto first_mem = head;
    while (first_mem->next->m_addr > first_mem->m_addr) first_mem = first_mem->next;
    first_mem = first_mem->next;

    // 寻找address的上一个空闲区节点以及下一个空闲区节点，分别存储于up_ptr，down_ptr
    map *up_ptr, *down_ptr;
    if (address <= first_mem->m_addr) {  // address上方没有空闲区的情况
        up_ptr = nullptr;
        down_ptr = first_mem;
    } else if (address >= first_mem->prior->m_addr) {  // address下方没有空闲区的情况
        up_ptr = first_mem->prior;
        down_ptr = nullptr;
    } else {  // address上下方都有空闲区的情况
        while (address >= first_mem->m_addr) first_mem = first_mem->next;
        up_ptr = first_mem->prior;
        down_ptr = first_mem;
    }

    // address上方没有空闲区，而address在申请空间外的情况，直接返回
    if (up_ptr == nullptr && address < p) {
        cout << "The area that needs to be released is outside the memory area!" << endl;
        return;
    }

    // address上方有空闲区，而address地址在上方空闲区内的情况，直接返回
    if (up_ptr != nullptr && address < up_ptr->m_addr + up_ptr->m_size) {
        cout << "Part of the area that needs to be released is in the free area!" << endl;
        return;
    }

    // address下方有空闲区，而需要释放的内存有部分在空闲区内的情况，直接返回
    if (down_ptr != nullptr && address + size > down_ptr->m_addr) {
        cout << "Part of the area that needs to be released is in the free area!" << endl;
        return;
    }

    // address下方无空闲区，而需要释放的内存有部分在申请空间外的情况，直接返回
    if (down_ptr == nullptr && address + size > p + MEMORY_SIZE) {
        cout << "The area that needs to be released is outside the memory area!" << endl;
        return;
    }

    // address上方无空闲区的内存释放情况
    if (up_ptr == nullptr) {
        // 需要释放的内存与下方空闲区不接触，则在空闲区链表中插入新的节点
        if (address + size < down_ptr->m_addr) {
            auto block = new map(size, address);
            block->prior = first_mem->prior;
            block->next = first_mem;
            first_mem->prior = block;
            block->prior->next = block;
            return;
        }

        // 需要释放的内存与下方空闲区合并，组成新的节点
        // 如果下方空闲区即为head指向的节点，则调整head的指向地址为新的空闲区首地址
        auto fake_head = first_mem;
        first_mem->m_addr = address;
        first_mem->m_size = first_mem->m_size + size;
        if (head == fake_head) head = first_mem;
        return;
    }

    // address下方无空闲区的内存释放情况
    if (down_ptr == nullptr) {
        // 需要释放的内存与上方空闲区不接触，则在空闲区链表中插入新的节点
        if (up_ptr->m_addr + up_ptr->m_size < address) {
            auto block = new map(size, address);
            block->prior = first_mem->prior;
            block->next = first_mem;
            first_mem->prior = block;
            block->prior->next = block;
            return;
        }

        // 需要释放的内存与上方空闲区合并，组成新的节点
        first_mem->prior->m_size = first_mem->prior->m_size + size;
        return;
    }

    // address上下均有空闲区的情况
    auto begin = up_ptr->m_addr + up_ptr->m_size;

    // address与上方空闲区与下方空闲区可以合并的情况，组成一个新的链表节点
    // 如果下方空闲区即为head指向的节点，则调整head的指向地址为新的空闲区首地址
    if (address == begin && address + size == down_ptr->m_addr) {
        up_ptr->m_size = up_ptr->m_size + size + down_ptr->m_size;
        auto temp_ptr = down_ptr->next;
        up_ptr->next = temp_ptr;
        temp_ptr->prior = up_ptr;
        if (head == down_ptr) head = up_ptr;
        delete down_ptr;  // 释放下方节点的空间
        return;
    }

    // address与上方空闲区可以合并(与下方节点无法合并)的情况，调整上方空闲区的大小
    if (address == begin) {
        up_ptr->m_size = up_ptr->m_size + size;
        return;
    }

    // address与下方空闲区可以合并(与上方节点无法合并)的情况，调整下方空闲区的大小和首地址
    // 如果下方空闲区即为head指向的节点，则调整head的指向地址为新的空闲区首地址
    if (address + size == down_ptr->m_addr) {
        auto fake_head = down_ptr;
        down_ptr->m_size = down_ptr->m_size + size;
        down_ptr->m_addr = address;
        if (head == fake_head) head = down_ptr;
        return;
    }

    // address与上下方空闲区均无法合并的情况，创建新的节点插入链表
    auto block = new map(size, address);
    block->prior = up_ptr;
    up_ptr->next = block;
    block->next = down_ptr;
    down_ptr->prior = block;
}

int main() {
    init();  // 初始化
    print();  // 打印空闲存储区情况
    char *ptr;

    ifstream in("../1.txt");
    string line;

    // 按行获得输入文件的内容
    while (getline(in, line)) {
        vector<string> res;
        string out;
        stringstream input(line);

        // 对于每一行的内容，按空格分割，存入数组res中
        while (input >> out) {
            res.emplace_back(out);
        }
        if (res[0] == "m") {  // 如果首字母是"m"，则进行内存分配
            ptr = lmalloc(unsigned(stoi(res[1])));

            if (ptr == nullptr) {  // 如果ptr指针为空，则说明内存分配失败
                cout << "Insufficient memory area" << endl;
                print();  // 打印空闲存储区情况
                continue;
            } else print();
        } else if (res[0] == "f") {  // 如果首字母是"f"，则进行内存释放
            lfree(unsigned(stoi(res[1])), unsigned(stoi(res[2])) + p);  // 输入的指针地址，为内存首地址+偏移
            print();  // 打印空闲存储区情况
        }
        cout << endl;
    }

    free(p);  // 释放分配的内存
    return 0;
}