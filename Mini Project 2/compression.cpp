/*
 * This is the main file for 2021S ACS Project 2.
 * Created by Yutong Xie and Yilu Zhou.
 */

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

#define BUFFER_SIZE 4096    // Define the buffer size.
typedef unsigned char BYTE; // Define BYTE as unsigned char.

// These variables will be assigned to different threads.
struct thread_data
{
    // The thread id for each thread in the loop [0..num_threads-1].
    long thread_id;
    // The 4K block of the input file.
    std::vector<BYTE> input_file_segment;
    // The result 4K block of the input file segment.
    std::vector<BYTE> result_file_segment;
};

// This function will read the file and convert the file into a vector
// of BYTE. Returns a std::vector<BYTE> of input file.
// input_file_name: name of the file.
std::vector<BYTE> read_file(const char *input_file_name)
{
    std::streampos file_size;
    std::ifstream input_file(input_file_name, std::ios::binary);

    // Get the file size.
    input_file.seekg(0, std::ios::end);
    file_size = input_file.tellg();
    std::cout << "input_file.size = " << file_size << " Bytes" << std::endl;
    input_file.seekg(0, std::ios::beg);

    // Read the file into the BYTE vector.
    std::vector<BYTE> file_data(file_size);
    input_file.read((char *)&file_data[0], file_size);

    // Close the input_file stream to save resources.
    input_file.close();
    return file_data;
}

// This function will covert the buffer to a vector of BYTE.
// vector: the destination vector.
// buffer: the input buffer.
// length: the size of the input buffer.
void buffer_vector_converter(std::vector<BYTE> &vector,
                          const BYTE *buffer,
                          uLongf length)
{
    for (int character_index = 0; character_index < length; character_index++)
    {
        BYTE current_character = buffer[character_index];
        vector.push_back(current_character);
    }
}

// This function will compress the vector using compress2 function.
// Returns the ret from compress2 function.
// source: the source vector containing the data from input_file.
// destination: the vector designed to hold the result.
int compress_vector(std::vector<BYTE> source, std::vector<BYTE> &destination)
{
    // Get the source length.
    unsigned long source_length = source.size();

    // Calculate the size of compressed vector.
    uLongf destination_length = compressBound(source_length);

    // Allocate memory to hold the compressed data.
    BYTE *destination_data = (BYTE *)malloc(destination_length);

    // If no memory space is allocated, return the error value.
    if (destination_data == nullptr)
    {
        return Z_MEM_ERROR;
    }

    // Format the source data.
    Bytef *source_data = (Bytef *)source.data();

    // Compress the source data using Z_BEST_COMPRESSION.
    int return_value = compress2((Bytef *)destination_data,
                                 &destination_length,
                                 source_data,
                                 source_length,
                                 Z_BEST_COMPRESSION);

    // Write buffer to the destination std::vector<BYTE>.
    buffer_vector_converter(destination, destination_data, destination_length);
    free(destination_data);
    return return_value;
}

// This function will decompress the vector using uncompress function.
// Returns the ret from uncompress function.
// source: the source vector containing the compressed data.
// destination: the destination space to hold the uncompressed data.
int decompress_vector(std::vector<BYTE> source, std::vector<BYTE> &destination)
{
    // Because each decompressed block is at most 4KB in size, create buffer
    // with 4KB memory space.
    uLongf destination_length = BUFFER_SIZE;
    BYTE *destination_data = (BYTE *)malloc(destination_length);

    // If no memory space is allocated, return the error value.
    if (destination_data == nullptr)
    {
        return Z_MEM_ERROR;
    }

    // Format the source data.
    Bytef *source_data = (Bytef *)source.data();
    int return_value = uncompress((Bytef *)destination_data,
                                  &destination_length,
                                  source_data,
                                  source.size());

    // Write buffer to the destination std::vector<BYTE>.
    buffer_vector_converter(destination, destination_data, destination_length);
    free(destination_data);
    return return_value;
}

// This is the function each thread will execute.
// threadargs: contains thread_id, input_file and compressed_file segments.
void *block_compression(void *threadargs)
{
    // Unpack the data from thread_data.
    struct thread_data *data;
    data = (struct thread_data *)threadargs;
    long tid = (long)data->thread_id;
    std::vector<BYTE> input_file_segment =
        (std::vector<BYTE>)data->input_file_segment;

    // Create result vector to hold the compressd block.
    std::vector<BYTE> result(0);

    // Call compress_vector to compress the 4KB block.
    int compression_result = compress_vector(input_file_segment, result);

    assert(compression_result == F_OK);

    // Write the result vector back to thread_data.
    data->result_file_segment = result;
    pthread_exit(NULL);
}

// This is the function used to decompress the input file.
// input: the input file in the format of std::vector<BYTE>.
// result: the decompressed file segment.
std::vector<BYTE> block_decompression(std::vector<BYTE> input)
{
    std::vector<BYTE> result(0);
    // Call decompress_vector.
    int decompression_result = decompress_vector(input, result);

    // Assert that the decompression process is correct.
    if (decompression_result == Z_BUF_ERROR)
    {
        std::cerr << "buffer error" << std::endl;
        exit(2);
    }
    if (decompression_result == Z_MEM_ERROR)
    {
        std::cerr << "mem error" << std::endl;
        exit(3);
    }
    if (decompression_result == Z_DATA_ERROR)
    {
        std::cerr << "data error" << std::endl;
        exit(4);
    }
    assert(decompression_result == F_OK);
    return result;
}

int main(int argc, char *argv[])
{
    // When compress mode.
    if (argc == 6 && argv[1] == std::string("-c"))
    {
        long num_threads = std::atoi(argv[5]);
        std::cout << "num_threads = " << num_threads << std::endl;
        if (num_threads == 0)
        {
            std::cerr << "invalid number of threads" << std::endl;
            exit(1);
        }
        // Enable the delivery of different data to and from different threads.
        struct std::vector<thread_data> thread_data_array(num_threads);
        char *input_file_name = argv[3];
        std::cout << "input_file = " << input_file_name << std::endl;
        std::ifstream input_file(argv[3], std::ios::binary);

        // Check the input_file.
        if (input_file.good() == false)
        {
            std::cerr << "cannot open input_file" << std::endl;
            exit(1);
        }

        // Call read_file to convert input_file to std::vector<BYTE>.
        std::vector<BYTE> input_file_data = read_file(argv[3]);
        remove(argv[4]); // Remove the output file from the directory.
        std::ofstream output_file(argv[4]);
        remove(argv[2]); // Remove the log file from the directory.
        std::ofstream log(argv[2]);
        std::cout << "output_file = " << argv[4] << std::endl;

        // Compress mode.
        std::cout << "compress mode: INPUT_FILE -> output_file + log_file"
                  << std::endl;
        // Dispatch the segment to different threads.
        pthread_attr_t attr;
        pthread_attr_init(&attr);
        pthread_attr_setdetachstate(&attr, PTHREAD_CREATE_JOINABLE);
        pthread_t threads[num_threads];
        void *status;
        long final = num_threads;
        bool flag = true;

        // Current start position of iterator of input_file vector.
        long long current_lower_bound = 0;
        // Current end position of iterator of input_file vector.
        long long current_upper_bound = current_lower_bound + BUFFER_SIZE;
        time_t start, end;
        start = time(NULL); // Start the clock when ready to compress.

        while (flag)
        {
            // Loop through [0..num_threads - 1].
            for (long thread_num = 0; thread_num < num_threads; thread_num++)
            {
                // The case when the end position is larger than the file size.
                if (current_upper_bound >= input_file_data.size())
                {
                    // The case when the start position is smaller than the
                    // file size.
                    if (current_lower_bound < input_file_data.size())
                    {
                        current_upper_bound = input_file_data.size();
                        final = thread_num + 1;
                    }
                    // The case when both are larger than the file size.
                    else
                    {
                        final = thread_num;
                        break;
                    }
                    flag = false; // Stop the loop.
                }

                std::vector<BYTE>::iterator first, last;
                first = input_file_data.begin() + current_lower_bound;
                last = input_file_data.begin() + current_upper_bound;
                std::vector<BYTE> buffer(first, last);
                thread_data_array[thread_num].thread_id = thread_num;
                thread_data_array[thread_num].input_file_segment = buffer;

                // Show basic information on the console window for the user to
                // estimate the progress.
                std::cout << "progress: "
                          << std::setprecision(3)
                          << float(current_lower_bound) / float(input_file_data.size()) * 100
                          << "% completed | thread "
                          << thread_num << ": "
                          << current_lower_bound << " - "
                          << current_upper_bound
                          << "          \r" << std::flush;

                // Create a thread with the thread_id and input_file_segment.
                int return_code = pthread_create(&threads[thread_num], &attr,
                                                 block_compression,
                                                 (void *)&thread_data_array[thread_num]);

                // When fail to create a thread, end the program.
                if (return_code)
                {
                    std::cout << "ERROR: return code from pthread_create() is "
                              << return_code << std::endl;
                    exit(3);
                }

                // Add the position of start and end position of iterators.
                current_lower_bound += BUFFER_SIZE;
                current_upper_bound += BUFFER_SIZE;
            }

            // Wait for num_threads threads to complete.
            for (long thread_num = 0; thread_num < final; thread_num++)
            {
                // Use pthread_join to wait for one thread to finish.
                int return_code = pthread_join(threads[thread_num], &status);

                // If the return_code is non-zero, end the program.
                if (return_code)
                {
                    std::cerr << "ERROR: return code from pthread_join() is "
                              << return_code << std::endl;
                    exit(3);
                }
            }

            // Write to the output_file the compressed file blocks in order.
            for (long thread_num = 0; thread_num < final; thread_num++)
            {
                std::vector<BYTE>::const_iterator i, j, k;
                i = thread_data_array[thread_num].result_file_segment.begin();
                j = thread_data_array[thread_num].result_file_segment.end();
                std::vector<BYTE> result(i, j);
                k = result.begin();
                for (; k != result.end(); k++)
                {
                    // Write the output_file with single character from the
                    // result_file_segment.
                    output_file << *k;
                }

                // Write to the log_file the size of result_file_segment.
                log << result.size() << std::endl;
            }
        }
        // Close log_file, input_file and output_file.
        log.close();
        input_file.close();
        output_file.close();
        std::cout << std::endl;
        std::cout << "done" << std::endl;

        // Show the user the total used.
        end = time(NULL);
        std::cout << "total_time = "
                  << difftime(end, start)
                  << "s"
                  << std::endl;
    }
    // When uncompress mode.
    // Uncompress mode will only use one thread to finish all the tasks.
    else if (argc == 5 && argv[1] == std::string("-u"))
    {
        // Similar to the cpmpress mode, check the input_file, output_file and
        // log_file.
        std::cout << "num_threads = 1" << std::endl;
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
        std::cout << "uncompress mode: input_file + log_file -> OUTPUT_FILE"
                  << std::endl;

        // current_pos will point to the start position of one compressed 4K
        // block.
        long long current_pos = 0;
        bool flag = true;
        time_t start, end;

        // Start the timer.
        start = time(NULL);

        while (flag)
        {
            long long length;
            log >> length; // Read one line from the log_file.

            // Calculate the end position of the compressed 4K block by adding
            // the current_pos with length.
            long long current_end = current_pos + length;

            // Ensure that the current_end will be smaller than the input_file.
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

            // Get one compressed 4K block from input_file.
            std::vector<BYTE>::iterator beg, end;
            beg = input_file_data.begin() + current_pos;
            end = input_file_data.begin() + current_pos + length;
            std::vector<BYTE> buffer(beg, end);

            // Show basic information on the console window for the user to
            // estimate the progress.
            std::cout << "progress: "
                      << std::setprecision(3)
                      << float(current_end) / float(input_file_data.size()) * 100
                      << "% completed | "
                      << current_pos << " - " << current_end
                      << "          \r" << std::flush;

            // Write the decompress result to the output_file.
            std::vector<BYTE> result = block_decompression(buffer);
            std::vector<BYTE>::iterator k = result.begin();
            for (; k != result.end(); k++)
            {
                output_file << *k;
            }
            current_pos = current_end;
        }

        // Close log_file, input_file and output_file.
        log.close();
        input_file.close();
        output_file.close();
        std::cout << std::endl;
        std::cout << "done" << std::endl;

        // Show the user the total used.
        end = time(NULL);
        std::cout << "total_time = "
                  << difftime(end, start)
                  << "s"
                  << std::endl;
    }
    // Possible future features.
    else
    {
        std::cerr << "./compression.out [-c] [log] [input] [output] [number_threads]"
                  << std::endl
                  << "./compression.out [-u] [log] [input] [output]"
                  << std::endl;
        pthread_exit(NULL);
        exit(1);
    }
    pthread_exit(NULL);
    return 0;
}