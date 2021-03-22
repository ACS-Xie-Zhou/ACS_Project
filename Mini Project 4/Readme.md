# Introduction
This mini-project implements a dictionary decoder. Dictionary decoding is widely used to compress data with low cardinality. The decoder will scan the original data to build the dictionary that consists all the unique data items, and then replaces each data item with the location ID in the dictionary. Hash table or B-tree are widely used indexing data structures to accelerate dictionary look-up. 
# Functions
The decoder in this mini project supports following functionalities:
- Encode: Read the raw data column and convert the data items to the location IDs in the dictionary. The dictionary and the encoded data column will be stored in the same file.
- Decode: Read the output from the encoding and convert the encoded data columns to the raw data column.
- Extract: Given the encoded data file and a location ID, find the raw data item.
- Hash: Given a random string, find the location ID of the given string in the dictionary.
- Query: Given the encoded data file and a string, check whether the string occurs in the raw data column. If yes, output the number of occurrences in the raw data column.
# Usage
***Linux is needed to run the program.***  
To compile the program, run  
`g++ main.cpp -o main.out`   
in the terminal window.  
After finishing compilation, to find all supported commands, run  
`./main.out`
## Encode
To run the encode mode, type  
`./main.out -e [original] [result]`  
in the terminal window.  
`[original]`: the raw data column. For demonstration, this will be `Large-Size-Column.txt`.  
`[result]`: the desired name of the output file. The output file is consisted of both the dictionary and encoded data column. For demonstration, this will be `result.txt`.  
Besides the `[original]` and `[result]`, `temp1` and `temp2` will be used by the program to store temporary information. ***Please ensure that there is no file named `temp1` or `temp2` in the same directory.*** Otherwise the file will be deleted by the program after running the encode mode.
## Decode
To run the decode mode, type  
`./main.out -d [result] [original]`  
in the terminal window.  
`[result]`: the encoded column file produced by the encode mode. For demonstration purpose, this will be `result.txt`.  
`[original]`: the desired name of the output decoded data file. For demonstration purpose, this will be `output.txt`.
## Extract
To run the extract mode, type  
`./main.out -x [result] [ID]`  
in the terminal window.  
`[result]`: the encoded column file produced by the encode mode. For demonstration purpose, this will be `result.txt`.  
`[ID]`: the location ID of the raw data item. This should be a unsigned integer. It is also possible that the location ID does not exist in the dictionary. The program will prompt the user for this as well.
## Hash
To run the hash mode, type  
`./main.out -h [value]`  
in the terminal window.  
`[value]`: the string the user wants to find the location ID from the dictionary. A unique location ID can be found for the string. It is possible that two different strings will generate the same location ID. This collision will be avoided by the program as well.
## Query
To run the query mode, type  
`./main.out -q [result] [value]`  
in the terminal window.  
`[result]`: the encoded column file produced by the encode mode. For demonstration purpose, this will be `result.txt`.  
`[value]`: the string the user wants to count the number of occurrences in the raw data column. The program will run query on the dictionary part of the encoded data column and count the occurrences for the user.
## Example
- If the user wants to encode the raw column file `Large-Size-Column.txt`, run `./main.out -e Large-Size-Column.txt result.txt` and the program will generate `result.txt` as the output file. 
- If the user wants to decode the column file `result.txt`, run `./main.out -d result.txt output.txt` and the program will decode the `result.txt` and output the decoded results to `output.txt`.
- If the user wants to find the corresponding string of a specific location ID, such as `8758215408589` in the encoded data column, run `./main.out -x result.txt 8758215408589` and the program will output `8758215408589 corresponds to bdeidgmj.` as the result.
- If the user wants to find the hash of a string, such as `nvhysbako`, run `./main.out -h nvhysbako` in the terminal window and the program will output `nvhysbako -> 282237731439478`, which means that the hash for the `nvhysbako` is `282237731439478`.
- If the user wants to count the number of occurrences of word `dugpnpzj` in the raw data column, run `./main.out -q result.txt dugpnpzj` and the program will output `dugpnpzj occurs 93 time(s).` as the result. If a non-existing string is used here, the program will output `Not Found`.

# Testing
## Configuration
- CPU: Intel Core i7-10700KF
- GPU: Nvidia GeForce 1660 Super
- RAM: 64GB DDR4 Memory
- OS:  Ubuntu 18.04 LTS 64-bit in VirtualBox
- Disk: Crucial CT1000MX500SSD1

## Results
### Encoding
| File Name | Size (KB) | Time (s) | Command |  
|---|---|---|---|
| Small-Size-Column.txt | 932 | 1 |./main.out -e Small-Size-Column.txt result_1.txt|
Medium-Size-Column.txt | 9382 | 2 | ./main.out -e Medium-Size-Column.txt result_2.txt |
Large-Size-Column.txt | 184132 | 37	|./main.out -e Large-Size-Column.txt result_3.txt|
![Encoding Results](./Picture1.svg)

### Query
| File Name | String | Time (s) | Command | result |
|-----------|--------|----------|---------|--------|
| result_1.txt | zlhwcbj | 1 | ./main.out -q result_1.txt zlhwcbj | zlhwcbj occurs 151 time(s) |
| result_1.txt | hzdcf | 1 | ./main.out -q result_1.txt hzdcf | hzdcf occurs 101 time(s) |
| result_1.txt | hfmy | 1 | ./main.out -q result_1.txt hfmy	|hfmy occurs 134 time(s) |
|result_2.txt|ynryjgfjmm|1|./main.out -q result_2.txt ynryjgfjmm|ynryjgfjmm occurs 102 time(s)|
|result_2.txt|fatkjwmku|1|./main.out -q result_2.txt fatkjwmku|fatkjwmku occurs 96 time(s)|
|result_2.txt|	kvha |1|./main.out -q result_2.txt kvha|kvha occurs 241 time(s)|
|result_3.txt|vcaysmqt|1|./main.out -q result_3.txt vcaysmqt|vcaysmqt occurs 122 time(s)|
|result_3.txt|wwyqldbstm|1|./main.out -q result_3.txt wwyqldbstm|wwyqldbstm occurs 118 time(s)|
|result_3.txt|kraqojygid|1|./main.out -q result_3.txt kraqojygid|kraqojygid occurs 113 time(s)|
![Query Results](./Picture2.svg)


