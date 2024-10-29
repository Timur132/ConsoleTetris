#include "include/ConsoleDisplay.h"

using namespace std;

int main() {
    ConsoleDisplay display{};
    auto prevTime = chrono::steady_clock::now();
    for(;;) {
        if(chrono::duration_cast<chrono::milliseconds>
        (chrono::steady_clock::now() - prevTime).count() >= 25) {
            display.tick();
            prevTime = chrono::steady_clock::now();
        }
	}
	return 0;
}
