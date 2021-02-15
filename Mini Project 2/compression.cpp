#include <iostream>
#include <fstream>
#include <zlib.h>
#include <pthread.h>
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <vector>

#define NUM_THREADS 32 // Define the number of threads.
#define CHUNK 4096
#define BUFFER_SIZE 4096 // Define the buffer size.
#define OUTPUT_SIZE 8192 // Define the output stream size.
typedef unsigned char BYTE;

struct thread_data
{
    long thread_id;
    std::vector<BYTE> input_file_segment;
    std::vector<BYTE> compressed_file_segment;
    unsigned char *dest;
};

struct thread_data thread_data_array[NUM_THREADS];

std::vector<BYTE> read_file(const char *input_file_name)
{
    std::streampos file_size;
    std::ifstream input_file(input_file_name, std::ios::binary);

    input_file.seekg(0, std::ios::end);
    file_size = input_file.tellg();
    std::cout << "input_file.size = " << file_size << std::endl;
    input_file.seekg(0, std::ios::beg);
    std::vector<BYTE> file_data(file_size);
    input_file.read((char *)&file_data[0], file_size);
    input_file.close();
    return file_data;
}

void *block_compression(void *threadargs)
{
    struct thread_data *data;
    data = (struct thread_data *)threadargs;
    long tid = (long)data->thread_id;
    std::vector<BYTE> seg_file = (std::vector<BYTE>)data->input_file_segment;
    BYTE *src = reinterpret_cast<BYTE *>(seg_file.data());
    //unsigned char *output = {0};
    //long unsigned int *output_size = sizeof(output);
    //*output_size = BUFFER_SIZE;
    //compress(output, output_size, src, BUFFER_SIZE);
    //std::cout << output << std::endl;

    data->compressed_file_segment = data->input_file_segment;
    pthread_exit(NULL);
}

int main(int argc, char *argv[])
{
    if (argc != 3)
    {
        // Ensure that the number of arguments is correct.
        std::cerr << "./compression.out [input] [output]"
                  << std::endl;
        pthread_exit(NULL);
        exit(0);
    }

    char *input_file_name = argv[1];
    std::cout << "input_file = " << input_file_name << std::endl;
    std::ifstream input_file(argv[1], std::ios::binary);
    if (input_file.good() == false)
    {
        std::cerr << "Cannot open input_file" << std::endl;
        exit(1);
    }
    std::vector<BYTE> input_file_data = read_file(argv[1]);
    remove(argv[2]); // Remove the output file from the directory.
    std::ofstream output_file(argv[2]);
    std::cout << "char.size = " << sizeof(char) << std::endl;
    std::cout << "BUFFER_SIZE = " << BUFFER_SIZE << std::endl;

    // Dispatch the segment to different threads.
    pthread_attr_t attr;
    pthread_attr_init(&attr);
    pthread_attr_setdetachstate(&attr, PTHREAD_CREATE_JOINABLE);
    pthread_t threads[NUM_THREADS];
    void *status;
    long final = NUM_THREADS;
    bool flag = true;
    long long current_lower_bound = 0;
    long long current_upper_bound = current_lower_bound + BUFFER_SIZE;

    while (flag)
    {
        for (long thread_num = 0; thread_num < NUM_THREADS; thread_num++)
        {

            if (current_upper_bound >= input_file_data.size())
            {
                if (current_lower_bound < input_file_data.size())
                {
                    current_upper_bound = input_file_data.size();
                    final = thread_num + 1;
                }
                else
                {
                    final = thread_num;
                    break;
                }
                flag = false;
            }
            std::cout << "thread " << thread_num << ": "
                      << current_lower_bound << " - "
                      << current_upper_bound << std::endl;

            std::vector<BYTE>::iterator first, last;
            first = input_file_data.begin() + current_lower_bound;
            last = input_file_data.begin() + current_upper_bound;
            std::vector<BYTE> buffer(first, last);
            thread_data_array[thread_num].thread_id = thread_num;
            thread_data_array[thread_num].input_file_segment = buffer;
            int return_code = pthread_create(&threads[thread_num], &attr,
                                             block_compression,
                                             (void *)&thread_data_array[thread_num]);
            if (return_code)
            {
                std::cout << "ERROR: return code from pthread_create() is "
                          << return_code << std::endl;
                exit(3);
            }
            current_lower_bound += BUFFER_SIZE;
            current_upper_bound += BUFFER_SIZE;
        }

        // Wait for these threads to complete.
        for (long thread_num = 0; thread_num < final; thread_num++)
        {
            int return_code = pthread_join(threads[thread_num], &status);
            if (return_code)
            {
                std::cout << "ERROR: return code from pthread_join() is "
                          << return_code << std::endl;
                exit(4);
            }
        }
        for (long thread_num = 0; thread_num < final; thread_num++)
        {
            std::vector<BYTE>::const_iterator i, j, k;
            i = thread_data_array[thread_num].compressed_file_segment.begin();
            j = thread_data_array[thread_num].compressed_file_segment.end();
            std::vector<BYTE> result(i, j);
            k = result.begin();
            for (; k != result.end(); k++)
            {
                output_file << *k;
            }
            //output_file << thread_data_array[thread_num].compressed_file_segment;
        }
    }
    output_file.close();
    pthread_exit(NULL);
    return 0;
}