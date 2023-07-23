#include "../zdunk/application.h"

int main(int argc, char **argvs)
{
    zdunk::Application app;
    if (app.init(argc, argvs))
    {
        return app.run();
    }
    return 0;
}