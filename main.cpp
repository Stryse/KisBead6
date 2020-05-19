#include <iostream>
#include <string>
#include "library/enumerator.hpp"
#include "library/summation.hpp"
#include "library/seqinfileenumerator.hpp"
#include "library/stringstreamenumerator.hpp"

// Struct Declarations
struct Trait;
struct Student;
struct Task;

// Istream overloads
std::istream& operator>>(std::ifstream& is, Trait& t);
std::istream& operator>>(std::istream& is, Student& st);
std::istream& operator>>(std::istream& is, Task& t);

// Infiles
const char* studentsInfile = "students2.txt";
const char* tasksInfile   = "tasks2.txt"; 

// Osszefuzi a stringeket amig nem talal szamot
class StringUntilNumSum : public Summation<std::string>
{
using Item = std::string;
using Value = Item;

public:
            Item getCurrentItem() const { return _enor->current(); } // Ebben adjuk vissza az id-t

protected:
            void first() override { }
            Value func(const Item& e) const override { return e; }
            Value neutral() const override { return ""; }
            Value add( const Value& a, const Value& b) const override { return a + b; }
            bool whileCond(const Item& current) const
            {
                try{
                     std::stoi(current);
                     return false;
                }
                catch(std::exception&)
                {
                    return true;
                }
            }
};

// Diak tulajdonsag tipus
struct Trait
{
    std::string t_name;
    double mul;

    static const double cheatMul;
    static const double lazyMul;
    static const double noCheatMul;
};
// Tulajdonsag szorzok
const double Trait::cheatMul   = 0.1;
const double Trait::lazyMul    = 0.5;
const double Trait::noCheatMul = 1.0;

// Diak tipus
struct Student
{
    int id;
    Trait trait;
};
std::istream& operator>>(std::istream& is, Student& st)
{
    // READ LINE
    std::string line;
    std::getline(is,line);
    std::stringstream ss(line);


    // SKIP NAME
    StringUntilNumSum readName;
    StringStreamEnumerator<std::string> reader(ss);
    reader.first();
    readName.addEnumerator(&reader);
    readName.run();

    // READ ID
    try{
            st.id = std::stoi(readName.getCurrentItem());
    }
    catch(std::exception&)
    {
            return is;
    }

    // READ TRAIT
    StringUntilNumSum readTrait;
    readTrait.addEnumerator(&reader);
    reader.next();
    readTrait.run();
    st.trait.t_name = readTrait.result();
    
         if("cheat" == st.trait.t_name)   st.trait.mul = Trait::cheatMul;
    else if("lazy"  == st.trait.t_name)   st.trait.mul = Trait::lazyMul;
    else if("nocheat" == st.trait.t_name) st.trait.mul = Trait::noCheatMul;

    return is;
}

// Feladat tipus
struct Task
{
    int s_id;
    int time;
};
std::istream& operator>>(std::istream& is, Task& t)
{
    // Read Line
    std::string line;
    std::getline(is,line);
    std::stringstream ss(line);
    ss >> t.s_id;

    // Skip name
    StringUntilNumSum readName;
    StringStreamEnumerator<std::string> reader(ss);
    reader.first();
    readName.addEnumerator(&reader);
    readName.run();

    // READ ID
    try{
            t.time = std::stoi(readName.getCurrentItem());
    } catch(std::exception&) {
            return is;
    }


    return is;
}

// Osszesiti a diakok jegyeit
class StudentMarkSum : public Summation<std::pair<int,int>,std::ostream>
{
using Item = std::pair<int,int>;
using Value = std::string;

protected:
            Value func(const Item& e) const override { return (std::to_string(e.first) + std::string(" ") + std::to_string(e.second)) + '\n'; }
public:
            StudentMarkSum(std::ostream* o) : Summation(o) {}
};

// Egy diak feladatait osszegzi
class TaskSum : public Summation<Task,int>
{
using Item = Task;
using Value = int;

private:
        const Student& curr_;
protected:
            void first() override {}
            Value func(const Item& e) const override { return e.time; }
            Value neutral() const override { return 0; }
            Value add( const Value& a, const Value& b) const override { return a + b; }
            bool whileCond(const Item& current) const override { return _enor->current().s_id == curr_.id; }
public:
        TaskSum(const Student& s) : curr_(s){
        }
};


// A diakokaz es a hozza tartozo feladatokat sorolja fel szinkron
class StudentMarkEnor : public Enumerator<std::pair<int,int>>
{
private:

         SeqInFileEnumerator<Student> students_;
         SeqInFileEnumerator<Task> tasks_;
         bool end_;
         std::pair<int,int> current_;

         int mapTimeToMark(double time) 
         {
                  if(time <= 10)             return 1;
             else if(time > 10 && time < 15) return 3;
             else if(time >= 15)             return 5;

             return -1;
          }

public:
        StudentMarkEnor(const std::string& st_file, const std::string& t_file) : students_(st_file), tasks_(t_file) {}
        virtual ~StudentMarkEnor() { }
        void first() override { students_.first(); tasks_.first(); next(); }
        void next() override
        {
            if(!(end_ = (students_.end() && tasks_.end())))
            {
                // Process
                TaskSum tSum(students_.current());
                tSum.addEnumerator(&tasks_);
                tSum.run();
                double tSumVal = tSum.result() * students_.current().trait.mul;
                current_ = std::pair<int,int>(students_.current().id, mapTimeToMark(tSumVal));

                // Iterate
                students_.next();
            }
        }
        std::pair<int,int> current() const override { return current_; }
        bool end() const override { return end_; }
};

int main()
{
    StudentMarkEnor marx(studentsInfile,tasksInfile);
    StudentMarkSum sum(&std::cout);
    sum.addEnumerator(&marx);
    sum.run();

    return 0;
}