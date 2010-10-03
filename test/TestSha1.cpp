#include <iostream>
#include <iomanip>
#include "../sha1/sha1.h"

int main()
{
    SHA1 sha;
    sha.Input("airtrack", 8);
    unsigned message_digest_array[5];
    if (!sha.Result(message_digest_array))
    {
        std::cerr << "error" << std::endl;
    }

    std::cout << std::hex
        << message_digest_array[0] << ' '
        << message_digest_array[1] << ' '
        << message_digest_array[2] << ' '
        << message_digest_array[3] << ' '
        << message_digest_array[4] << std::endl;

    return 0;
}
