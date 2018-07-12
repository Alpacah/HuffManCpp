#include <algorithm>
#include <iostream>
#include <fstream>
#include <vector>
#include <string>
#include <map>
#include <fstream>

using namespace std;

// CUSTOM TYPE //
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

// SORTING FUNCTIONS //
bool sort_tree(Node* a, Node* b){
    if (a->frequence != b->frequence){
        return a->frequence < b->frequence;
    }else{
        return a->value < b->value;
    }
}

bool sort_huffcodes(pair<unsigned char, string> a, pair<unsigned char, string> b){
    if ((a.second).length() != (b.second).length()){
        return a.second.length() < b.second.length();
    }else{
        return a.first < b.first;
    }
}

// ALGORITHM //
string str_to_bin(string str){
    string out;
    short int counter = sizeof(unsigned char) * 8 - 1;
    unsigned char mask;

    // Encode content
    for (unsigned int i = 0; i < str.length(); i++){
        mask = (str.at(i) == '1') ? (mask | (1 << counter)) : (mask & ~(1 << counter));
        counter -= 1;

        if (counter == -1){
            counter = sizeof(unsigned char) * 8 - 1;
            out += mask;
        }
    }
    out += mask;

    // Get last byte mask
    string last_byte_mask;
    mask = counter;
    last_byte_mask += mask;

    return last_byte_mask + out;
}

void set_codes(Node* root, string str, map<unsigned char, string>* huff){
    if (!root){
        return;
    }else{
        if (root->value != '$'){
            (*huff)[root->value] = str;
            cout << root->value << ": " << str << endl;
        }
        set_codes(root->leftChild, str + "0", huff);
        set_codes(root->rightChild, str + "1", huff);
    }
}

// FILES OPERATIONS //
string get_content(string path){
    string result;
    ifstream file(path, ios::in | ios::binary);

    if (file){
        string line;
        while (getline(file, line)){
            result += line + '\n';
        }
        result.erase(prev(result.end()));

        return result;
    }else{
        return "";
    }
}

void write_file(string path, string content){
    ofstream file(path, ios::out | ios::binary);
    file << content;
    file.close();
}

int main(){
    cout << "-- HUFFMAN CPP --" << endl;
    cout << "File to treat: ";
    string path;
    cin >> path;

    string content = get_content(path);

    if (content != ""){
        // Define mode
        int mode = (path.substr(path.find_last_of(".") + 1) == "huff");
        cout << (mode ? "Decoding" : "Encoding") << " mode" << endl;

        if (mode == 0){
            // Get frequencies
            map<unsigned char, unsigned int> frequencies;
            for (unsigned int i = 0; i < content.length(); i++){
                unsigned char actual_char = content.at(i);
                frequencies[actual_char] = (frequencies.find(actual_char) == frequencies.end()) ? 1 : (frequencies[actual_char] + 1);
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
            set_codes(tree[0], "", &huffcodes);

            // ENCODING //
            string encoded_string;

            // Header
            string header;
            unsigned char header_char_count = huffcodes.size();
            for (int i = sizeof(unsigned char) * 8 - 1; i >= 0; i--){
                header += ((header_char_count & (1 << i)) != 0) ? '1' : '0';
            }

            for (map<unsigned char, string>::const_iterator it = huffcodes.begin(); it != huffcodes.end(); it++){
                // symbol - 1 byte
                for (int i = sizeof(unsigned char) * 8 - 1; i >= 0; i--){
                    header += ((it->first & (1 << i)) != 0) ? '1' : '0';
                }

                // code length - 1 byte
                unsigned char code_length = (it->second).length();
                for (int i = sizeof(unsigned char) * 8 - 1; i >= 0; i--){
                    header += ((code_length & (1 << i)) != 0) ? '1' : '0';
                }

                // code - code_length bits
                for (int i = 0; i < (it->second).length(); i++){
                    header += (it->second).at(i);
                }
            }
            cout << "Bin header: " << (header.length()/8) << " bytes" << endl;
            encoded_string += header;

            // Body
            string body;
            for (unsigned int i = 0; i < content.size(); i++){
                body += huffcodes[content.at(i)];
            }
            cout << "Body: " << (body.length()/8) << " bytes" << endl;
            encoded_string += body;

            // Save
            string encoded_path = path + ".huff";
            write_file(encoded_path, str_to_bin(encoded_string));
            cout << "Encoded as " << encoded_path << endl;
        }else{
            // MODE DECODE
            unsigned char filling_bits = content.at(0);
            content.erase(0, 1);
            unsigned char dico_size = content.at(0);
            content.erase(0, 1);
            map<string, unsigned char> huffcodes;

            // Get decoded binary
            string decoded_binary;
            for (unsigned int i = 0; i < content.length(); i++){
                unsigned char actual_char = content.at(i);
                for (int c = sizeof(unsigned char) * 8 - 1; c >= 0; c--){ // Don't put c as unsigned because it throws a bad alloc erro
                    decoded_binary += ((actual_char & (1 << c)) != 0) ? "1" : "0";;
                }
            }

            // Decode header
            int cursor = 0;
            for (int i = 0; i < dico_size; i++){
                // Get symbol
                unsigned char symbol;
                for (int c = 0; c < sizeof(unsigned char) * 8; c++){
                    symbol = (decoded_binary.at(cursor + c) == '1') ? (symbol | (1 << (7 - c))) : (symbol & ~(1 << (7 - c)));
                }
                cursor += 8;

                // Code length
                unsigned char code_length;
                for (int c = 0; c < sizeof(unsigned char) * 8; c++){
                    code_length = (decoded_binary.at(cursor + c) == '1') ? (code_length | (1 << (7 - c))) : (code_length & ~(1 << (7 - c)));
                }
                cursor += 8;

                // Code
                string code;
                for (int c = 0; c < code_length; c++){
                    code += decoded_binary.at(cursor + c);
                }
                cursor += code_length;
                huffcodes[code] = symbol;
                cout << symbol << ": " << code << endl;
            }

            // Decode body
            string tmp, decoded_string;
            tmp = "";
            for (unsigned int c = cursor; c < decoded_binary.length() - filling_bits; c++){
                tmp += decoded_binary[c];
                if (huffcodes.find(tmp) != huffcodes.end()){
                    decoded_string += huffcodes[tmp];
                    tmp = "";
                }
            }

            // Save decoded
            string decoded_path = path;
            decoded_path.erase(decoded_path.end() - 5, decoded_path.end());
            write_file(decoded_path, decoded_string);
            cout << "Decoded as " << decoded_path << endl;
        }
    }else{
        cerr << "Failed to open file " << path << endl;
    }

    system("PAUSE");

    return 0;
}
