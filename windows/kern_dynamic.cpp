#include <string>
#include "kern_dynamic.h"

void kern_dynamic::insert_section(std::string sec_name, uint64_t sh_offset, uint64_t sh_size)
{
}

int kern_dynamic::ksym_dlsym(const char* newString, size_t* out_address)
{
    int result = -1;
    return result;
}

int kern_dynamic::parseAndGetGlobals()
{
    int result = -1;
    return result;

}