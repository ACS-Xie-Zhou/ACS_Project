#include <iostream>
#include <string>
#include <unordered_map>
#include <utility>
#include <vector>
#include <algorithm>
#include <fstream>
#include <ctime>

typedef unsigned long long int key_type;
typedef std::string val_type;

typedef int occurrence_type;
// Create a pair to hold both the string and occurrence to accelerate search.
typedef std::pair<val_type, occurrence_type> pair_type;
// Create a vector in case of collision.
typedef std::vector<pair_type> item;

// Create a dictionary to store vector of pairs.
typedef std::unordered_map<key_type, item> dict_container;
// Create a dictionary to decode the file.
typedef std::unordered_map<val_type, val_type> decode_container;

// Function to generate hash for given value.
// Ideally no duplicate values will be given by the program.
// val: the string object.
// Returns the long long int as hash value.
key_type hash_function(const val_type &val)
{
    // The hash function from Java hashCode().
    key_type hash = 7;
    int c;
    for (int i = 0; i < val.length(); i++)
    {
        hash = hash * 31 + val[i];
    }
    return hash;
}

// Function to insert the string object to the dictionary, with occurrence
// counted.
// val: the string object.
// dict: the dictionary object.
// log: the ofstream to output log for reference.
void insert(const val_type &val, dict_container &dict,
            std::ofstream &output)
{
    // Find the hash value.
    unsigned long long int hash = hash_function(val);
    dict_container::iterator itr;
    itr = dict.find(hash); // Check whether unique in dictionary, O(1).
    if (itr == dict.end())
    {
        // Ensure there is no collision for this pair.
        item new_item;

        // Insert a pair at the hash table with hash as key.
        new_item.push_back(std::make_pair(val, 1));
        dict.insert(std::make_pair(hash, new_item));
    }
    else
    {
        // There is collision.
        item::iterator item_itr;
        // Worst case: O(N)
        for (item_itr = itr->second.begin(); item_itr != itr->second.end(); item_itr++)
        {
            if (item_itr->first == val)
            {
                // If there is record of the same raw data item in the
                // dictionary,
                // increment the second item in the pair by 1.
                item_itr->second++;
                break;
            }
        }
        if (item_itr == itr->second.end())
        {
            // If there is no record of the same raw data item in the
            // dictionary,
            // insert a pair at the hash table with hash as key.
            pair_type new_pair = std::make_pair(val, 1);
            itr->second.push_back(new_pair);
        }
    }
    output << hash << std::endl; // Write the encoded data column.
}

int main(int argc, char *argv[])
{
    clock_t start, end;
    start = clock();
    if (argc == 4 && argv[1] == std::string("-e"))
    {
        // Encode Mode.
        std::cout << "Encode Mode" << std::endl;
        char *input_file_name = argv[2];
        const char *output_file_name = "temp1";
        const char *dictionary_file_name = "temp2";
        char *result_file_name = argv[3];

        dict_container dict;
        std::ofstream output;
        std::ofstream dictionary;
        output.open(output_file_name);
        dictionary.open(dictionary_file_name);

        std::ifstream infile(input_file_name);
        std::string one_line;
        std::cout << "Please Wait..." << std::endl;
        while (infile >> one_line)
        {
            // Read all the lines from the input file.
            insert(one_line, dict, output);
        }
        dict_container::const_iterator itr;

        for (itr = dict.begin(); itr != dict.end(); itr++)
        {
            // Record all the items in the dictionary data structure.
            item::const_iterator item_itr;
            for (item_itr = itr->second.begin(); item_itr != itr->second.end(); item_itr++)
            {
                dictionary << itr->first << " " << item_itr->first << " "
                           << item_itr->second << std::endl;
            }
        }
        output.close();
        dictionary.close();

        // Combine the two files.
        std::ifstream if_output(output_file_name, std::ios_base::binary);
        std::ifstream if_dict(dictionary_file_name, std::ios_base::binary);
        std::ofstream of_result(result_file_name, std::ios_base::binary);

        of_result << if_dict.rdbuf() << "00000"
                  << " EOF "
                  << "00000"
                  << std::endl
                  << if_output.rdbuf();
        of_result.close();

        remove(output_file_name);
        remove(dictionary_file_name);

        std::cout << "Done" << std::endl;
        end = clock();
        double endtime = (double)(end - start) / CLOCKS_PER_SEC;
        std::cout << "total_time = "
                  << endtime * 1000
                  << "ms"
                  << std::endl;
    }

    else if (argc == 4 && argv[1] == std::string("-d"))
    {
        // Decode Mode.
        std::cout << "Decode Mode" << std::endl;
        char *input_file_name = argv[2];
        char *output_file_name = argv[3];
        std::string s1, s2, s3;
        decode_container decoder;
        std::ofstream output;
        output.open(output_file_name);

        std::ifstream infile(input_file_name);

        // Read the dictionary part of the encoded data column.
        while (infile >> s1 >> s2 >> s3)
        {
            if (s1 == std::string("00000") && s2 == std::string("EOF") && s3 == std::string("00000"))
            {
                // If separator is found, exit loop.
                break;
            }
            decoder[s1] = s2; // Reconstruct the dictionary in memory.
        }
        std::cout << "Decoder Reconstructed" << std::endl;
        std::cout << "Please Wait..." << std::endl;
        decode_container::const_iterator itr;

        // Read the remaining item, convert back to raw data item.
        while (infile >> s1)
        {
            itr = decoder.find(s1);
            output << itr->second << std::endl;
        }
        output.close();
        std::cout << "Done" << std::endl;
        end = clock();
        double endtime = (double)(end - start) / CLOCKS_PER_SEC;
        std::cout << "total_time = "
                  << endtime * 1000
                  << "ms"
                  << std::endl;
    }

    else if (argc == 4 && argv[1] == std::string("-x"))
    {
        // Extract Mode.
        std::cout << "Extract Mode" << std::endl;
        char *input_file_name = argv[2];
        char *input_string = argv[3];
        std::ifstream infile(input_file_name);
        std::string s1, s2, s3;
        std::cout << "Please Wait..." << std::endl;

        // Read the dictionary part of the encoded data column.
        while (infile >> s1 >> s2 >> s3)
        {
            if (s1 == std::string("00000") && s2 == std::string("EOF") && s3 == std::string("00000"))
            {
                break;
            }
            // If there is an entry with the same location ID, output the string.
            if (s1 == std::string(input_string))
            {
                std::cout << input_string << " corresponds to " << s2
                          << std::endl;
                std::cout << "Done" << std::endl;
                end = clock();
                double endtime = (double)(end - start) / CLOCKS_PER_SEC;
                std::cout << "total_time = "
                          << endtime * 1000
                          << "ms"
                          << std::endl;
                return 0;
            }
        }

        // If not found, prompt the user.
        std::cout << "Invalid Location ID" << std::endl;
        std::cout << "Done" << std::endl;
        end = clock();
        double endtime = (double)(end - start) / CLOCKS_PER_SEC;
        std::cout << "total_time = "
                  << endtime * 1000
                  << "ms"
                  << std::endl;
    }

    else if (argc == 3 && argv[1] == std::string("-h"))
    {
        // Hash Mode.
        std::cout << "Hash Mode" << std::endl;
        char *input_string = argv[2];
        // Send the string to the hash function, output the result.
        std::cout << input_string << " -> " << hash_function(input_string)
                  << std::endl;
        std::cout << "Done" << std::endl;
        end = clock();
        double endtime = (double)(end - start) / CLOCKS_PER_SEC;
        std::cout << "total_time = "
                  << endtime * 1000
                  << "ms"
                  << std::endl;
    }

    else if (argc == 4 && argv[1] == std::string("-q"))
    {
        // Query Mode.
        std::cout << "Query Mode" << std::endl;
        char *input_file_name = argv[2];
        char *input_string = argv[3];
        // Get the hash value.
        key_type hash_val = hash_function(input_string);
        std::ifstream infile(input_file_name);
        std::string s1, s2, s3;
        std::cout << "Please Wait..." << std::endl;

        // Read the dictionary region of the encoded data column.
        // Find the entry with the same hash value.
        // Output the third item of the entry.
        while (infile >> s1 >> s2 >> s3)
        {
            if (s1 == std::string("00000") && s2 == std::string("EOF") && s3 == std::string("00000"))
            {
                break;
            }
            if (s1 == std::to_string(hash_val) && s2 == std::string(input_string))
            {
                std::cout << input_string << " occurs " << s3 << " time(s)"
                          << std::endl;
                std::cout << "Done" << std::endl;
                end = clock();
                double endtime = (double)(end - start) / CLOCKS_PER_SEC;
                std::cout << "total_time = "
                          << endtime * 1000
                          << "ms"
                          << std::endl;
                return 0;
            }
        }
        std::cout << "Not Found" << std::endl;
        std::cout << "Done" << std::endl;
        end = clock();
        double endtime = (double)(end - start) / CLOCKS_PER_SEC;
        std::cout << "total_time = "
                  << endtime * 1000
                  << "ms"
                  << std::endl;
    }

    else
    {
        // Additional info.
        std::cout << "Encode  ./main.out -e [original] [result]"
                  << std::endl;
        std::cout << "Decode  ./main.out -d [result] [original]" << std::endl;
        std::cout << "Extract ./main.out -x [result] [ID]" << std::endl;
        std::cout << "Hash    ./main.out -h [value]" << std::endl;
        std::cout << "Query   ./main.out -q [result] [value]" << std::endl;
    }
}
