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

bool sort_huffcodes(pair<unsigned char, string> a, pair<unsigned char, string> b){
    if ((a.second).length() != (b.second).length()){
        return a.second.length() < b.second.length();
    }else{
        return a.first < b.first;
    }
}

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
        }
        set_codes(root->leftChild, str + "0", huff);
        set_codes(root->rightChild, str + "1", huff);
    }
}

void print_encode_huffcodes(map<unsigned char, string> huffcodes){
    vector<pair<unsigned char, string>> huffcodes_vector;
    for (map<unsigned char, string>::const_iterator it = huffcodes.begin(); it != huffcodes.end(); it++){
        huffcodes_vector.push_back(make_pair(it->first, it->second));
    }

    sort(huffcodes_vector.begin(), huffcodes_vector.end(), sort_huffcodes);

    for (vector<pair<unsigned char, string>>::const_iterator it = huffcodes_vector.begin(); it != huffcodes_vector.end(); it++){
        if (it->first == '\n'){
            cout << "\\n";
        }else if (it->first == '\r'){
            cout << "\\r";
        }else{
            cout << it->first;
        }
        cout << ": " << it->second << endl;
    }
}

void print_decode_huffcodes(map<string, unsigned char> huffcodes){
    vector<pair<unsigned char, string>> huffcodes_vector;
    for (map<string, unsigned char>::const_iterator it = huffcodes.begin(); it != huffcodes.end(); it++){
        huffcodes_vector.push_back(make_pair(it->second, it->first));
    }

    sort(huffcodes_vector.begin(), huffcodes_vector.end(), sort_huffcodes);

    for (vector<pair<unsigned char, string>>::const_iterator it = huffcodes_vector.begin(); it != huffcodes_vector.end(); it++){
        if (it->first == '\n'){
            cout << "\\n";
        }else if (it->first == '\r'){
            cout << "\\r";
        }else{
            cout << it->first;
        }
        cout << ": " << it->second << endl;
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
        content.erase(prev(content.end()));
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
            print_encode_huffcodes(huffcodes);

            // Output
            ofstream outfile("out.txt", ios::out | ios::binary);

            // Header
            string header;
            for (map<unsigned char, string>::const_iterator it = huffcodes.begin(); it != huffcodes.end(); it++){
                header += it->first;
                header += it->second;
            }
            outfile << header;

            // Body
            string binstring;
            for (unsigned int i = 0; i < content.size(); i++){
                binstring += huffcodes[content.at(i)];
                cout << content.at(i);
            }
            binstring = str_to_bin(binstring);
            outfile << '$' << binstring; // $ for the end of the header
            outfile.close();

            float compression_ratio = (float) (binstring.length() + header.length()) / (float) content.length();
            cout << "Compression successful, " << content.length() << " bytes -> " << (binstring.length() + header.length()) << " bytes" << endl << "Compression ratio: " << (compression_ratio * 100) << "%" << endl;
        }else{
            // MODE DECODE
            map<string, unsigned char> huffcodes;
            unsigned char header_char = '$';
            unsigned char last_byte_mask = '$';
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
                            }
                            header_char = actual_char;
                            tmp = "";
                        }
                    }else{
                        huffcodes[tmp] = header_char;
                        print_decode_huffcodes(huffcodes);
                        decode_pos = 1;
                    }
                }else{
                    // BODY
                    if (last_byte_mask == '$'){
                        // Get last byte mask
                        last_byte_mask = actual_char;
                    }else{
                        // Decode
                        for (int c = sizeof(unsigned char) * 8 - 1; c >= 0; c--){ // Don't put c as unsigned because it throws a bad alloc erro
                            body += ((actual_char & (1 << c)) != 0) ? "1" : "0";;
                        }
                    }
                }
            }

            // Decode body
            string output;
            tmp = "";
            for (unsigned int c = 0; c < body.length() - last_byte_mask; c++){
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
