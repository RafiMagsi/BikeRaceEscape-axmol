#include "AppDelegate.h"
#include "PZLegacyCompat.h"

int main(int argc, char** argv) {
    (void)argc;
    (void)argv;
    AppDelegate app;
    return ax::Application::getInstance()->run();
}
