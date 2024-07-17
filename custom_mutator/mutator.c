
#include "afl-fuzz.h"
#include <stdlib.h>

#define DATA_SIZE 0x7FFFFF
#define RANDOM_IF(code_1, code_2) if (rand() % 2) {code_1;} else {code_2;}

enum DE_data_type
{
        BYTE     = 1,
        ASCII    = 2,
        INT      = 3,
        LONG     = 4,
        RATIONAL = 5
};

enum Attribute_tagID
{
        IMAGE_WIDTH = 0x0100, 	 
	IMAGE_HIGH  = 0x0101
};


typedef struct my_custom_mutator
{
        afl_state_t *afl;
        uint8_t *mutated_out;
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
        // there are the array of TIFF_DE
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
        
        if ((data->mutated_out = (uint8_t*)malloc(MAX_FILE)) == NULL) 
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
        free(data->mutated_out);
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
                       uint8_t **out_buf, uint8_t *add_buf,
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

        if (offset < sizeof(TIFF_IFH))
                return offset;

        RANDOM_IF(buffer[0] = 0x49, buffer[0] = 0x4d)   // 4949 - little-endian
        RANDOM_IF(buffer[1] = 0x49, buffer[1] = 0x4d)   // 4d4d - big-endian

        RANDOM_IF(buffer[2] = 0x00, buffer[2] = 0x2a)   // 0x2a - TIFF flag
        RANDOM_IF(buffer[3] = 0x00, buffer[3] = 0x2a)

        RANDOM_IF(, *(int*) (buffer + 4) = rand();)

        // from sizeof(TIFF_IFH) to offset -- pixel image data
        RANDOM_IF(, for (size_t i = sizeof(TIFF_IFH); i < offset; i++){buffer[i] &= (rand() % 255);})

        TIFF_IFD* first_IFD = (TIFF_IFD*) (buffer + offset);
        size_t DE_count = first_IFD->DE_count;

        if (offset + DE_count * sizeof(TIFF_DE) + sizeof(TIFF_IFD) <= mutated_size)
        {
                RANDOM_IF(first_IFD->DE_count = (unsigned short) rand(), )

                TIFF_DE* current = (TIFF_DE*) (buffer + offset + sizeof(first_IFD->DE_count));

                for (size_t DE_index = 0; DE_index < DE_count; DE_index++)
                {
                        if (current->tag == IMAGE_WIDTH || current->tag == IMAGE_HIGH)
                        {
                                current->type   = INT;
                                current->length = 1;
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