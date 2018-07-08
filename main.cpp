#include <algorithm>
#include <iostream>
#include <fstream>
#include <vector>
#include <string>
#include <map>
#include <fstream>


using namespace std;

struct Node{
    unsigned char value;
    unsigned int frequence;
    Node* leftChild;
    Node* rightChild;

    Node(unsigned char m_value, unsigned int m_frequence){
        leftChild = rightChild = NULL;
        value = m_value;
        frequence = m_frequence;
    }
};

bool sort_tree(Node* a, Node* b){
    if (a->frequence != b->frequence){
        return a->frequence < b->frequence;
    }else{
        return a->value < b->value;
    }
}

string get_bin_outstring(string str){
    string out;
    int counter = sizeof(unsigned char) * 8 - 1;
    unsigned char mask;

    for (unsigned int i = 0; i < str.length(); i++){
        if (str.at(i) == '1'){
            mask |= (1 << counter);
        }else{
            mask &= ~(1 << counter);
        }
        counter -= 1;
        if (counter == -1){
            counter = sizeof(unsigned char) * 8 - 1;
            out += mask;
        }
    }

    return out;
}

void print_code(Node* root, string str, map<unsigned char, string>* huff){
    if (!root){
        return;
    }else{
        if (root->value != '$'){
            cout << root->value << ": " << str << endl;
            (*huff)[root->value] = str;
        }
        print_code(root->leftChild, str + "0", huff);
        print_code(root->rightChild, str + "1", huff);
    }
}


int main(){
    cout << "-- HUFFMAN DU PAUVRE --" << endl;
    cout << "Fichier a traiter: ";
    string path;
    cin >> path;

    ifstream file;
    file.open(path, ios::in | ios::binary);

    if (file){
        // Read file
        string line, content;
        while (getline(file, line)){
            content += line + '\n';
        }
        file.close();

        // Choose mode
        cout << "[0]: ENCODE | [1]: DECODE" << endl;
        int mode;
        cin >> mode;

        if (mode == 0){
            // MODE ENCODE
            map<unsigned char, unsigned int> frequencies;
            for (unsigned int i = 0; i < content.length(); i++){
                unsigned char actual_char = content.at(i);
                if (frequencies.find(actual_char) == frequencies.end()){
                    frequencies[actual_char] = 0;
                }
                frequencies[actual_char] += 1;
            }

            // Build tree
            vector<Node*> tree;
            for (map<unsigned char, unsigned int>::const_iterator it = frequencies.begin(); it != frequencies.end(); it++){
                tree.push_back(new Node(it->first, it->second));
            }
            while (tree.size() != 1){
                Node* left = tree[0];
                tree.erase(tree.begin());
                Node* right = tree[0];
                tree.erase(tree.begin());

                Node* top = new Node('$', left->frequence + right->frequence);
                top->leftChild = left;
                top->rightChild = right;
                tree.push_back(top);

                sort(tree.begin(), tree.end(), sort_tree);
            }

            // Print tree and get the associative array
            map<unsigned char, string> huffcodes;
            print_code(tree[0], "", &huffcodes);

            // Output
            ofstream outfile("out.txt", ios::out | ios::binary);
            // Header
            for (map<unsigned char, string>::const_iterator it = huffcodes.begin(); it != huffcodes.end(); it++){
                outfile << it->first << it->second;
            }
            // Body
            string binstring;
            for (unsigned int i = 0; i < content.size(); i++){
                binstring += huffcodes[content.at(i)];
            }
            outfile << '$' << get_bin_outstring(binstring); // $ for the end of the header
            outfile.close();
        }else{
            // MODE DECODE
            map<string, unsigned char> huffcodes;
            char header_char = '$';
            string tmp, body;
            int decode_pos = 0; // 0: header | 1: body
            for (unsigned int i = 0; i < content.length(); i++){
                unsigned char actual_char = content.at(i);

                if (decode_pos == 0){
                    // HEADER
                    if (actual_char != '$'){
                        if (actual_char == '1' || actual_char == '0'){
                            tmp += actual_char;
                        }else{
                            if (header_char != '$'){
                                huffcodes[tmp] = header_char;
                                cout << header_char << ": " << tmp << endl;
                            }
                            header_char = actual_char;
                            tmp = "";
                        }
                    }else{
                        huffcodes[tmp] = header_char;
                        decode_pos = 1;
                    }
                }else{
                    // BODY
                    //cout << endl << "[" << i << "/" << content.length() << "] " << actual_char << ": ";
                    for (int c = sizeof(unsigned char) * 8 - 1; c >= 0; c--){
                        body += ((actual_char & (1 << c)) != 0) ? "1" : "0";;
                    }
                }
            }

            // Decode body
            string output;
            tmp = "";
            for (int c = 0; c < body.length(); c++){
                tmp += body[c];
                if (huffcodes.find(tmp) != huffcodes.end()){
                    output += huffcodes[tmp];
                    tmp = "";
                }
            }
            cout << output << endl;
        }
    }else{
        cerr << "Failed to open file" << endl;
    }

    return 0;
}