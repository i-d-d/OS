#include "./headers/interface.h"

int main() {
    command input;
    do {
        input = get_command();
        execute_command(input);
    } while (input != EXIT);
    return 0;
}