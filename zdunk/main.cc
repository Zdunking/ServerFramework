#include "application.h"
#include <cstdlib>
#include <ctime>

int main(int argc, char **argvs)
{
    setenv("TZ", ":/etc/localtime", 1);
    tzset();
    srand(time(0));
    zdunk::Application app;
    if (app.init(argc, argvs))
    {
        return app.run();
    }
    return 0;
}