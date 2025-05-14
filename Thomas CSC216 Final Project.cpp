#include <cstdlib>
#include <cctype>
#include <iostream>
#include <fstream>
#include <sstream>
#include <utility>
#include <map>
#include <list>
#include <vector>
#include <string>
#include <algorithm>

using namespace std;

const long MAX_FNAME = 200;

int wagner_fischer(string s1, string s2)
{
    int len_s1 = s1.length(), len_s2 = s2.length();
    int d[len_s1+1][len_s2+1];
    for(int i = 0; i <= len_s1; i++)
    {
        d[i][0] = i;
    }
    for(int j = 0; j <= len_s2; j++)
    {
        d[0][j] = j;
    }
    for(int i = 1; i <= len_s1; i++)
    {
        for(int j = 1; j <= len_s2; j++)
        {
            if(s1[i-1] == s2[j-1])
            {
                d[i][j] = d[i-1][j-1];
            }
            else
            {
                int temp = min((d[i-1][j]),(d[i][j-1]));
                d[i][j] = 1 + min(temp,(d[i-1][j-1]));
            }
        }
    }
    return d[len_s1][len_s2];
    
}

template <typename E>
class BKTree{
    protected:
    class Node {
        public:
        E element;
        Node* parent;
        map<int, Node*> children;
  
        Node(E e, Node* p = nullptr) : element{e}, parent{p} {}
    };
    Node* rt{nullptr};
    int sz{0};

    public:
    BKTree(){}

    Node* sentinel() const {return rt;}

    int size() const {return sz;}
    bool empty() const {return sz == 0;}

    void add_root(const E& e = E()) {               // add root to (presumed) empty tree
        rt = new Node(e);
        sz = 1;
    }

    void insert_node(const E& e)
    {
        Node* temp = rt;
        bool inserted = false;
        while (!inserted)
        {
        int key = wagner_fischer(temp->element, e);
        if(!temp->children.empty() && temp->children.count(key) != 0)
        {
            temp = temp->children.at(key);
        }
        else
        {
            temp->children[key] = new Node{e, temp};
            sz++;
            inserted = true;
        }
        }
    }

    list<pair<int,E>> lookup(const E& word, int margin, Node* nd)
    {
        Node* temp_root = nd;
        int rt_wf = wagner_fischer(word, temp_root->element);
        list<pair<int,E>> matches;
        if(rt_wf == 0)
        {
            matches.push_back(make_pair(0,temp_root->element));
        }
        else
        {
            matches.push_back(make_pair(rt_wf,temp_root->element));
            int i = max(1, rt_wf - margin);
            for(i; i <= rt_wf + margin; i++)
            {
                if(temp_root->children.count(i) != 0)
                {
                    matches.merge(lookup(word, margin, temp_root->children.at(i)));
                }
            }
        }  
        matches.sort();
        if(matches.empty())
        {
            //this is just so it doesn't throw a segmentation fault. It'll just return the word.
            //Doesn't properly spell check it but it's better than crashing
            matches.push_back(make_pair(1,word));
        }
        return matches;
    }
};

BKTree<string> get_dict(const string& tableFileName)
{
    BKTree<string> dict;
    dict.add_root("the");
    string line;
    bool end_reached = false;

    ifstream inFile(tableFileName);
    if (!inFile)
    {
        inFile.close();
        inFile.clear();
        return dict;
    }

    while (!end_reached){
        getline(inFile,line);
        if (line == "-----")
        {
            end_reached = true;
        }
        else
        {
            istringstream iss(line); 
            string word;
            if (iss >> word)
            {
                dict.insert_node(word);
            }
        }
    }
    inFile.close();

    return dict;
}

string lookup_fast(const string& str, BKTree<string> tree)
{
    return tree.lookup(str, 2, tree.sentinel()).front().second;
}

void getline(char str[], long MAX, istream & in = cin)
{
    if (in.tie() != nullptr)
    {
        in.tie()->flush();
    }
    while (in.good() && in.peek() == '\n')
    {
        in.ignore();
    }
    in.getline(str, MAX);
    return;
}
//ifstream open
void open(ifstream & in, char fname[], long MAX_FN)
{
    cout << "Enter input file name:  ";
    getline(fname, MAX_FN);
    in.open(fname);
    while (!in)
    {
        in.close();
        in.clear();
        cout << "\nInvalid file name, file must exist (and be readable)!\nEnter input file name:  ";
        getline(fname, MAX_FN);
        in.open(fname);
    }
    return;
}

bool was_writable(ofstream & out)
{
    bool opened = true;
    if (!out)
    {
        opened = false;
        out.close();
        out.clear();
        cout << "\nInvalid file name:  file location must be writable!\n";
    }
    return opened;
}
//ofstream open
void open(ofstream & out, char fname[], long MAX_FN)
{
    bool opened;
    ifstream test_exists;
    do
    {
        opened = true;
        cout << "Enter output file name:  ";
        getline(fname, MAX_FN);
        test_exists.open(fname);
        if (test_exists)
        {
            test_exists.close();
            test_exists.clear();
            cout << "\nInvalid file name:  file must NOT exist!\n";
            opened = false;
        }
        else
        {
            test_exists.close();
            test_exists.clear();
            out.open(fname);
            opened = was_writable(out);
        }
    } while (!opened);
    return;
}

void spell_check(istream & in, ostream & out, BKTree<string> dict)
{
    char ch;
    string word;

    while(in.get(ch))
    {
        if(ch == ' ' || ch == '\n' || ch == '\t' || (ispunct(ch) && ch != '\'') || isdigit(ch))
        {
            if(!word.empty())
            {
                out << lookup_fast(word, dict);
                word.clear();
            }
            out << ch;
        }
        else
        {
            word += tolower(ch);
        }
    }
    if(!word.empty())
    {
        out << lookup_fast(word, dict);
    }
    return;
}

int main(void)
{
    BKTree<string> dictionary;

    dictionary = get_dict("20kwordlist.txt");

    ifstream in_file;
    ofstream out_file;
    char fname[MAX_FNAME];

    open(in_file, fname, MAX_FNAME);
    open(out_file, fname, MAX_FNAME);
    spell_check(in_file, out_file, dictionary);
    in_file.close();
    out_file.close();

    return 0;
}