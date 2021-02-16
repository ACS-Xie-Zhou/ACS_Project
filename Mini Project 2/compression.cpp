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

#define NUM_THREADS 16   // Define the number of threads.
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
    //std::cout << "finished" << std::endl;
}

int compress_vector(std::vector<BYTE> source, std::vector<BYTE> &destination)
{
    unsigned long source_length = source.size();
    //unsigned long source_length = BUFFER_SIZE;
    uLongf destination_length = compressBound(source_length);
    //uLongf destination_length = OUTPUT_SIZE;

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

// int decompress_vector(std::vector<BYTE> source, std::vector<BYTE> &destination)
// {
//     std::cout << "begin decompress" << std::endl;
//     unsigned long source_length = source.size();
//     std::cout << "source.size = " << source_length << std::endl;
//     uLongf destination_length = compressBound(source_length);

//     BYTE *destination_data = (BYTE *)malloc(destination_length);

//     if (destination_data == nullptr)
//     {
//         return Z_MEM_ERROR;
//     }

//     Bytef *source_data = (Bytef *)source.data();
//     int return_value = uncompress((Bytef *)destination_data, &destination_length, source_data, source.size());
//     add_buffer_to_vector(destination, destination_data, destination_length);
//     free(destination_data);
//     std::cout << "end decompress" << std::endl;
//     std::cout << "source.size = " << source.size() << std::endl;
//     std::cout << "dest.size = " << destination.size() << std::endl;
//     std::cout << "ret = " << return_value << std::endl;
//     return return_value;
// }

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

// void *block_uncompression(void *threadargs)
// {
//     struct thread_data *data;
//     data = (struct thread_data *)threadargs;
//     long tid = (long)data->thread_id;
//     std::vector<BYTE> input_file_segment = (std::vector<BYTE>)data->input_file_segment;
//     std::vector<BYTE> result(0);
//     int decompression_result = decompress_vector(input_file_segment, result);
//     std::cout << decompression_result << std::endl;

//     if (decompression_result == Z_BUF_ERROR)
//     {
//         std::cerr << "Buffer Error." << std::endl;
//         exit(1);
//     }
//     if (decompression_result == Z_MEM_ERROR)
//     {
//         std::cerr << "MEM Error." << std::endl;
//         exit(2);
//     }
//     if (decompression_result == Z_DATA_ERROR)
//     {
//         std::cerr << "Data Error." << std::endl;
//         exit(3);
//     }
//     assert(decompression_result == F_OK);
//     data->compressed_file_segment = result;
//     pthread_exit(NULL);
// }

int main(int argc, char *argv[])
{
    // if (argc != 4 || (argv[1] != std::string("-c") && argv[1] != std::string("-u")))
    // {
    //     // Ensure that the number of arguments is correct.
    //     std::cerr << "./compression.out [-c/-u] [input] [output]"
    //               << std::endl;
    //     pthread_exit(NULL);
    //     exit(0);
    // }

    // char *input_file_name = argv[2];
    // std::cout << "input_file = " << input_file_name << std::endl;
    // std::ifstream input_file(argv[2], std::ios::binary);
    // if (input_file.good() == false)
    // {
    //     std::cerr << "Cannot open input_file" << std::endl;
    //     exit(1);
    // }
    // std::vector<BYTE> input_file_data = read_file(argv[2]);
    // remove(argv[3]); // Remove the output file from the directory.
    // std::ofstream output_file(argv[3]);
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
    std::cout << "output_file = " << argv[2] << std::endl;

    if (true)
    {
        // Compress mode.
        std::cout << "compress mode: INPUT_FILE -> output_file" << std::endl;
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
                //std::cout << result.size() << std::endl;
                k = result.begin();
                for (; k != result.end(); k++)
                {
                    output_file << *k;
                }
                //output_file << thread_data_array[thread_num].compressed_file_segment;
                //output_file << std::endl;
            }
            //output_file << "This is the end of file.";
        }
        //
        output_file.close();
        std::cout << "                                                                    \r"
                  << std::flush;

        std::cout << "done" << std::endl;
        clock_t end = clock();
        double time_duration = static_cast<double>(end - start) / CLOCKS_PER_SEC;
        std::cout << "total_time = " << time_duration << "s" << std::endl;
    }
    // else if (argv[1] == std::string("-u"))
    // {
    //     // Uncompress mode.
    //     std::cout << "uncompress mode: input_file -> OUTPUT_FILE" << std::endl;

    //     pthread_attr_t attr;
    //     pthread_attr_init(&attr);
    //     pthread_attr_setdetachstate(&attr, PTHREAD_CREATE_JOINABLE);
    //     pthread_t threads[NUM_THREADS];
    //     void *status;

    //     std::string one_line;
    //     long thread_num = 0;
    //     bool flag = true;

    //     while (flag)
    //     {
    //         flag = false;
    //         while (getline(input_file, one_line))
    //         {
    //             if (one_line.compare("This is the end of file.") == 0)
    //             {
    //                 // End of file reached.
    //                 break;
    //             }
    //             //std::cout << thread_num << ": " << one_line << std::endl;

    //             std::vector<BYTE> buffer(one_line.begin(), one_line.end());
    //             thread_data_array[thread_num].thread_id = thread_num;
    //             thread_data_array[thread_num].input_file_segment = buffer;
    //             int return_code = pthread_create(&threads[thread_num], &attr,
    //                                              block_uncompression,
    //                                              (void *)&thread_data_array[thread_num]);
    //             if (return_code)
    //             {
    //                 std::cout << "ERROR: return code from pthread_create() is "
    //                           << return_code << std::endl;
    //                 exit(3);
    //             }
    //             flag = true;
    //             if (thread_num < NUM_THREADS - 1)
    //             {
    //                 thread_num++;
    //             }
    //             else
    //             {
    //                 thread_num = NUM_THREADS;
    //                 break;
    //             }
    //         }
    //         //std::cout << "thread_num = " << thread_num << std::endl;
    //         for (long i = 0; i < thread_num; i++)
    //         {
    //             //std::cout << "Recycled " << i << std::endl;
    //             int return_code = pthread_join(threads[i], &status);
    //             if (return_code)
    //             {
    //                 std::cout << "ERROR: return code from pthread_join() is "
    //                           << return_code << std::endl;
    //                 exit(4);
    //             }
    //         }
    //         for (long i = 0; i < thread_num; i++)
    //         {
    //             std::vector<BYTE>::const_iterator beg, end, k;
    //             beg = thread_data_array[i].compressed_file_segment.begin();
    //             end = thread_data_array[i].compressed_file_segment.end();
    //             std::vector<BYTE> result(beg, end);
    //             k = result.begin();
    //             for (; k != result.end(); k++)
    //             {
    //                 output_file << *k;
    //             }
    //             //output_file << std::endl;
    //         }
    //         thread_num = 0;
    //     }
    //     output_file.close();
    // }
    // else
    // {
    //     std::cout << "no operation" << std::endl;
    // }
    pthread_exit(NULL);
    return 0;
}