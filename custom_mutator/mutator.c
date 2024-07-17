
#include "../../../AFL__/AFLplusplus/include/afl-fuzz.h"
#include <X11/X.h>
#include <stdlib.h>

#define DATA_SIZE 8388607
#define RANDOM_IF(code_1, code_2) if (rand() % 2) {code_1;} else {code_2;}

typedef struct my_custom_mutator
{
        afl_state_t *afl;

        // any additional data here!
        size_t trim_size_current;
        int    trimmming_steps;
        int    cur_step;

  u8 *mutated_out, *post_process_buf, *trim_buf;
} my_custom_mutator_t;

typedef struct 
{
	unsigned short  tiff_magic;      /* magic number (defines byte order) */
	unsigned short  tiff_version;    /* TIFF version number */
	unsigned int    tiff_diroff;     /* byte offset to first directory */

} TIFF_IFH; // Image File Header


typedef struct 
{
        unsigned short  tag;
        unsigned short  type;
        unsigned int    length;
        unsigned int    value_offset;

} TIFF_DE; // Directory Entry 

typedef struct 
{
        unsigned short DE_count;
        TIFF_DE DEs[1];

        unsigned int next_IFD_offset;

} TIFF_IFD; // Image File Directory 

size_t mutate_tiff_file(unsigned char * buffer, size_t mutated_size);


/**
* Initialize the custom mutator.
*
* @param afl AFL instance.
* @param seed Seed used for the mutation.
* @return pointer to internal data or NULL on error
*/
   
void *afl_custom_init(afl_state_t *afl, unsigned int seed)
{
        srand(seed);  // needed also by surgical_havoc_mutate()

        my_custom_mutator_t* data = NULL;
        
        data = (my_custom_mutator_t*) calloc(1, sizeof(my_custom_mutator_t));
        if (!data)
        {
                perror("afl_custom_init: allocation error");
                return NULL;
        }
        
        if ((data->mutated_out = (u8 *)malloc(MAX_FILE)) == NULL) 
        {
                perror("afl_custom_init: allocation error");
                return NULL;

        }

        if ((data->post_process_buf = (u8 *)malloc(MAX_FILE)) == NULL) 
        {
                perror("afl_custom_init: allocation error");
                return NULL;

        }

        if ((data->trim_buf = (u8 *)malloc(MAX_FILE)) == NULL) 
        {
                perror("afl_custom_init: allocation error");
                return NULL;
        }


        data->afl = afl;

        return data;
}




/**
* Deinitialize the custom mutator.
*
* @param data pointer returned in afl_custom_init by this custom mutator
*/

void afl_custom_deinit(my_custom_mutator_t *data) 
{
        free(data->post_process_buf);
        free(data->mutated_out);
        free(data->trim_buf);
        free(data);
}


/**
* Perform custom mutations on a given input
*
* (Optional)
*
* Getting an add_buf can be skipped by using afl_custom_splice_optout().
*
* @param[in] data Pointer returned in afl_custom_init by this custom mutator
* @param[in] buf Pointer to the input data to be mutated and the mutated
*     output
* @param[in] buf_size Size of the input/output data
* @param[out] out_buf The new buffer, under your memory mgmt.
* @param[in] add_buf Buffer containing an additional test case (splicing)
* @param[in] add_buf_size Size of the additional test case
* @param[in] max_size Maximum size of the mutated output. The mutation must
* not produce data larger than max_size.
* @return Size of the mutated output.
*/

size_t afl_custom_fuzz(my_custom_mutator_t *data, uint8_t *buf, size_t buf_size,
                       u8 **out_buf, uint8_t *add_buf,
                       size_t add_buf_size,  // add_buf can be NULL
                       size_t max_size)
{

        // Make sure that the packet size does not exceed the maximum size expected by
        // the fuzzer

        size_t mutated_size = DATA_SIZE <= max_size ? DATA_SIZE : max_size;

        memcpy(data->mutated_out, buf, buf_size);

        mutate_tiff_file(data->mutated_out, mutated_size);
        
        if (mutated_size > max_size) {mutated_size = max_size;}

        *out_buf = data->mutated_out;
        return mutated_size;
}

size_t mutate_tiff_file(unsigned char * buffer, size_t mutated_size)
{
        TIFF_IFH* hdr = (TIFF_IFH*) buffer;

        size_t offset = hdr->tiff_diroff <= mutated_size ? hdr->tiff_diroff : mutated_size;

        RANDOM_IF(buffer[0] = 0x49, buffer[0] = 0x4d)
        RANDOM_IF(buffer[1] = 0x49, buffer[1] = 0x4d)

        RANDOM_IF(buffer[2] = 0x00, buffer[2] = 0x2a)
        RANDOM_IF(buffer[3] = 0x00, buffer[3] = 0x2a)

        RANDOM_IF(, unsigned int tmp = rand(); memcpy(buffer + 4, (unsigned char*) &tmp, 4))

        // from sizeof(TIFF_IFH) to offset -- pixel image data
        RANDOM_IF(, for (size_t i = sizeof(TIFF_IFH); i < offset; i++){buffer[i] &= (rand() % 255);})
        

        TIFF_IFD* first_IFD = (TIFF_IFD*) (buffer + offset);
        size_t DE_count = first_IFD->DE_count;



        if (offset + DE_count * sizeof(TIFF_DE) + 6 <= mutated_size)
        {
                RANDOM_IF(first_IFD->DE_count = (unsigned short) rand(), )

                TIFF_DE* current = (TIFF_DE*) (buffer + offset + 2);

                for (size_t DE_index = 0; DE_index < DE_count; DE_index++)
                {
                        if (current->tag == 0x0100 || current->tag == 0x0101)
                        {
                                current->type = 0x03;
                                current->length = 0x1;
                                current->value_offset = rand();
                                continue;
                        }
                        RANDOM_IF(current->tag          = rand(),)
                        RANDOM_IF(current->type         = rand(),)
                        RANDOM_IF(current->length       = rand(),)
                        RANDOM_IF(current->value_offset = rand(),)
                        current += 1;
                }
        }
        else
        {
                for (size_t i = offset; i < mutated_size; i++)
                {
                        buffer[i] &= (rand() % 255);
                }
        }

        

        

        return offset;
}