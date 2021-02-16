#include <iostream>
#include <fstream>
#include <zlib.h>
#include <pthread.h>
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <vector>
#include <cassert>
#include <iomanip>
#include <ctime>

#define NUM_THREADS 8    // Define the number of threads.
#define BUFFER_SIZE 4096 // Define the buffer size.
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
    std::cout << "input_file.size = " << file_size << " Bytes" << std::endl;
    input_file.seekg(0, std::ios::beg);
    std::vector<BYTE> file_data(file_size);
    input_file.read((char *)&file_data[0], file_size);
    input_file.close();
    return file_data;
}

void add_buffer_to_vector(std::vector<BYTE> &vector, const BYTE *buffer, uLongf length)
{
    for (int character_index = 0; character_index < length; character_index++)
    {
        BYTE current_character = buffer[character_index];
        vector.push_back(current_character);
    }
}

int compress_vector(std::vector<BYTE> source, std::vector<BYTE> &destination)
{
    unsigned long source_length = source.size();
    uLongf destination_length = compressBound(source_length);

    BYTE *destination_data = (BYTE *)malloc(destination_length);
    if (destination_data == nullptr)
    {
        return Z_MEM_ERROR;
    }

    Bytef *source_data = (Bytef *)source.data();
    int return_value = compress2((Bytef *)destination_data, &destination_length, source_data, source_length,
                                 Z_BEST_COMPRESSION);
    add_buffer_to_vector(destination, destination_data, destination_length);
    free(destination_data);
    return return_value;
}

int decompress_vector(std::vector<BYTE> source, std::vector<BYTE> &destination)
{
    unsigned long source_length = source.size();
    uLongf destination_length = BUFFER_SIZE;
    BYTE *destination_data = (BYTE *)malloc(destination_length);
    if (destination_data == nullptr)
    {
        return Z_MEM_ERROR;
    }

    Bytef *source_data = (Bytef *)source.data();
    int return_value = uncompress((Bytef *)destination_data, &destination_length, source_data, source.size());
    add_buffer_to_vector(destination, destination_data, destination_length);
    free(destination_data);
    return return_value;
}

void *block_compression(void *threadargs)
{
    struct thread_data *data;
    data = (struct thread_data *)threadargs;
    long tid = (long)data->thread_id;
    std::vector<BYTE> input_file_segment = (std::vector<BYTE>)data->input_file_segment;
    std::vector<BYTE> result(0);
    int compression_result = compress_vector(input_file_segment, result);
    assert(compression_result == F_OK);

    data->compressed_file_segment = result;
    pthread_exit(NULL);
}

std::vector<BYTE> block_uncompression(std::vector<BYTE> input)
{
    std::vector<BYTE> result(0);
    int decompression_result = decompress_vector(input, result);
    //std::cout << decompression_result << std::endl;

    if (decompression_result == Z_BUF_ERROR)
    {
        std::cerr << "Buffer Error." << std::endl;
        exit(1);
    }
    if (decompression_result == Z_MEM_ERROR)
    {
        std::cerr << "MEM Error." << std::endl;
        exit(2);
    }
    if (decompression_result == Z_DATA_ERROR)
    {
        std::cerr << "Data Error." << std::endl;
        exit(3);
    }
    assert(decompression_result == F_OK);
    return result;
}

int main(int argc, char *argv[])
{
    if (argc != 5 || (argv[1] != std::string("-c") && argv[1] != std::string("-u")))
    {
        // Ensure that the number of arguments is correct.
        std::cerr << "./compression.out [-c/-u] [log] [input] [output]"
                  << std::endl;
        pthread_exit(NULL);
        exit(0);
    }
    if (argv[1] == std::string("-c"))
    {
        char *input_file_name = argv[3];
        std::cout << "input_file = " << input_file_name << std::endl;
        std::ifstream input_file(argv[3], std::ios::binary);
        if (input_file.good() == false)
        {
            std::cerr << "Cannot open input_file" << std::endl;
            exit(1);
        }
        std::vector<BYTE> input_file_data = read_file(argv[3]);
        remove(argv[4]); // Remove the output file from the directory.
        std::ofstream output_file(argv[4]);
        remove(argv[2]);
        std::ofstream log(argv[2]);
        std::cout << "output_file = " << argv[4] << std::endl;

        // Compress mode.
        std::cout << "compress mode: INPUT_FILE -> output_file + log_file" << std::endl;
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

        clock_t start = clock();
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

                std::cout << "process: "
                          << std::setprecision(3)
                          << float(current_lower_bound) / float(input_file_data.size()) * 100
                          << "% completed | thread "
                          << thread_num << ": "
                          << current_lower_bound << " - "
                          << current_upper_bound
                          << "              \r" << std::flush;

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
                log << result.size() << std::endl;
            }
        }
        log.close();
        input_file.close();
        output_file.close();
        // std::cout << "                                                                    \r"
        //           << std::flush;
        std::cout << std::endl;
        std::cout << "done" << std::endl;
        clock_t end = clock();
        double time_duration = static_cast<double>(end - start) / CLOCKS_PER_SEC;
        std::cout << "total_time = " << time_duration << "s" << std::endl;
    }
    else if (argv[1] == std::string("-u"))
    {
        char *input_file_name = argv[3];
        char *log_file_name = argv[2];
        std::cout << "input_file = " << input_file_name << std::endl;
        std::ifstream input_file(argv[3], std::ios::binary);
        if (input_file.good() == false)
        {
            std::cerr << "Cannot open input_file" << std::endl;
            exit(1);
        }
        std::vector<BYTE> input_file_data = read_file(argv[3]);
        std::cout << "log_file = " << log_file_name << std::endl;
        std::ifstream log(argv[2]);
        remove(argv[4]); // Remove the output file from the directory.
        std::ofstream output_file(argv[4]);
        std::cout << "output_file = " << argv[4] << std::endl;
        std::cout << "uncompress mode: input_file + log_file -> OUTPUT_FILE" << std::endl;
        std::string one_line;
        long thread_num = 0;
        long long current_pos = 0;
        bool flag = true;

        while (flag)
        {
            long long length;
            log >> length;
            long long current_end = current_pos + length;
            if (current_end >= input_file_data.size())
            {
                if (current_pos < input_file_data.size())
                {
                    current_end = input_file_data.size();
                }
                else
                {
                    break;
                }
                flag = false;
            }
            std::vector<BYTE>::iterator beg = input_file_data.begin() + current_pos;
            std::vector<BYTE>::iterator end = input_file_data.begin() + current_pos + length;
            std::cout << "process: "
                      << std::setprecision(3)
                      << float(current_end) / float(input_file_data.size()) * 100
                      << "% completed | " 
                      << current_pos << " - " << current_end 
                      << "              \r" << std::flush;

            //std::cout << current_pos << " - " << current_end << std::endl;
            std::vector<BYTE> buffer(beg, end);
            std::vector<BYTE> result = block_uncompression(buffer);
            std::vector<BYTE>::iterator k = result.begin();
            for (; k != result.end(); k++)
            {
                output_file << *k;
            }
            current_pos = current_end;
        }
        std::cout << std::endl;
        std::cout << "done" << std::endl;
        std::cout << "output_file = " << argv[4] << std::endl;
        log.close();
        input_file.close();
        output_file.close();
    }
    else
    {
        std::cout << "no operation" << std::endl;
        exit(0);
    }
    pthread_exit(NULL);
    return 0;
}