//
// Created by Alexey Konstantinov on 07.05.2023.
//

#ifndef TESTTASK_INITI_TABLE_H
#define TESTTASK_INITI_TABLE_H

#include <deque>
#include <mutex>
#include <iostream>
#include <string>
#include <algorithm>

class Table;
class Row;


typedef class StringColumn{
private:
    std::string value_;
public:

    explicit StringColumn(std::string& value) noexcept{
        this->value_ = std::move(value);
    }

    explicit StringColumn(char* value) noexcept{
        this->value_ = value;
    }
    explicit StringColumn(const char* value) noexcept{
        this->value_ = value;
    }
    explicit StringColumn() noexcept {
        this->value_ = "";
    };
    ~StringColumn()=default;

    [[nodiscard]] const std::string& get_value() const{
        return this->value_;
    }

    StringColumn& operator=(char* value) noexcept{
        this->value_ = value;
        return *this;
    }
    StringColumn& operator=(std::string& value) noexcept{
        this->value_ = value;
        return *this;
    }
    StringColumn& operator=(const char* value) noexcept{
        this->value_ = value;
        return *this;
    }

    friend std::ostream& operator<< (std::ostream &os, const StringColumn &sc) noexcept{
        return os << sc.value_;
    }

    friend bool operator<(const StringColumn &a, const StringColumn &b) noexcept{
        return (a.value_.compare(b.value_) < 0);
    }

} StringColumn;

typedef uint_fast32_t Identifier; // Just for the demo. UUID (for example) should be used in real case

typedef class Row{
public:
    Row(const Identifier &id, const char *column_1, const char *column_2) noexcept{
        id_ = id;
        column_1_ = column_1;
        column_2_ = column_2;
        initialized_ = true;
    }
    Row(const Identifier &id, std::string &column_1, std::string &column_2) noexcept{
        id_ = id;
        column_1_ = column_1;
        column_2_ = column_2;
        initialized_ = true;
    }
    Row(const Identifier &id, std::string column_1, std::string column_2) noexcept{
        id_ = id;
        column_1_ = column_1;
        column_2_ = column_2;
        initialized_ = true;
    }

    Row()=default;
    ~Row()=default;

    [[nodiscard]] const Identifier& get_id() const noexcept{
        return this->id_;
    }
    [[nodiscard]] const StringColumn& get_column_1() const noexcept{
        return this->column_1_;
    }
    [[nodiscard]] const StringColumn& get_column_2() const noexcept{
        return this->column_2_;
    }

    [[nodiscard]] bool is_initialized() const noexcept{
        return this->initialized_;
    }

    friend std::ostream& operator<<(std::ostream& os, const Row &r) noexcept{
        if (r.initialized_)
            return os << r.id_ << " " << r.column_1_ << " " << r.column_2_;
        else
            return os;
    }

private:
    Identifier id_{0};
    StringColumn column_1_;
    StringColumn column_2_;
    bool initialized_{false};


    friend Table;

    static bool compare_by_id (const Row &a, const Row &b) noexcept{
        return a.id_ < b.id_;
    }
    static bool compare_by_column_1 (const Row &a, const Row &b) noexcept{
        return a.column_1_ < b.column_1_;
    }
    static bool compare_by_column_2 (const Row &a, const Row &b) noexcept{
        return a.column_2_ < b.column_2_;
    }

} Row;

typedef enum {
    ID,
    COLUMN_1,
    COLUMN_2
} ColumnName;


typedef class Table{

public:

    explicit Table(std::deque<Row> data) noexcept{
        this->data_ = std::move(data);
        this->max_id_ = 0;
    }
    Table()=default;
    ~Table()=default;

    Row get_element(Identifier id) noexcept{

        this->m.lock();
        for (const auto &r : data_){
            if (r.id_ == id){
                this->m.unlock();
                return r;
            }
        }
        this->m.unlock();
        return {};
    }

    bool remove_element(Identifier id){
        auto p = this->data_.begin();
        while (p != this->data_.end()){
            if(p->id_ == id){
                this->data_.erase(p);
                return true;
            }
        }
        return false; // There was no such element
    }

    bool insert_element(Row& row, unsigned int pos){
        this->m.lock();
        try {
            row.id_ = this->get_unique_id();
            this->data_.emplace(data_.begin() + pos, row);
            this->m.unlock();
            return true;
        } catch (std::exception &ex){
            std::cout << ex.what() << std::endl;
            this->m.unlock();
            return false;
        }
    };

    bool emplace_back(const char *column_1, const char *column_2){
        try {
            Identifier id = this->get_unique_id();
            Row row(id, column_1, column_2);
            this->m.lock();
            this->data_.emplace_back(row);
            this->m.unlock();
            return true;
        } catch (std::runtime_error &er){
            std::cout << er.what() << std::endl;
            return false;
        }
    }
    bool emplace_back(Row &row){
        try {
            Identifier id = this->get_unique_id();
            row.id_ = id;
            this->m.lock();
            this->data_.emplace_back(row);
            this->m.unlock();
            return true;
        } catch (std::runtime_error &er){
            std::cout << er.what() << std::endl;
            return false;
        }
    }


    Identifier get_unique_id(){
        /*
         * Demo usage for fast uint32 identifiers
         */

        if (max_id_ == UINT_FAST32_MAX){
            throw std::runtime_error("Maximum identifier has been reached");
        }
        return ++max_id_;
    }


    std::deque<Row> get_sorted_range(int from, int N, ColumnName column){
        auto sorter = Table::get_sorter(column);
        auto kth = data_.begin() + from - 1;
        auto last = std::min(kth + N, data_.end());

        m.lock();
        std::nth_element(data_.begin(),kth, data_.end(), sorter);
        std::partial_sort(kth, last, data_.end(), sorter);
        std::deque<Row> result(kth, last);
        m.unlock();

        return result;
    };

    Row operator[](unsigned int index){
        try {
            this->m.lock();
            auto result = this->data_.at(index);
            this->m.unlock();
            return result;
        } catch (std::out_of_range &ex){
            m.unlock();
            std::cout << "Index " << index << " is out of the range" << std::endl;
            return {};
        } catch (...){
            m.unlock();
            std::cout << "Unexpected exception" << std::endl;
            return {};
        }
    }

    friend std::ostream& operator<< (std::ostream &os, const Table &tb){
        for (const auto &r : tb.data_){
            os << r << std::endl;
        }
        return os;
    }
private:
    std::deque<Row> data_;
    std::mutex m;
    Identifier max_id_{0};

    static bool (*get_sorter(ColumnName name))(const Row&, const Row&){
        bool (*sorter)(const Row &, const Row &);
        switch(name){
            case ID:
                sorter = Row::compare_by_id;
                break;
            case COLUMN_1:
                sorter = Row::compare_by_column_1;
                break;
            case COLUMN_2:
                sorter = Row::compare_by_column_2;
                break;
        }
        return sorter;
    };

} Table;



#endif //TESTTASK_INITI_TABLE_H
