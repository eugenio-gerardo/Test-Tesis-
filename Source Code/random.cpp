#include "random.h"
#include <cmath>
double u_random()
{
    return double(rand())/RAND_MAX;
}
double u_random(double const &a, double const &b)
{
    return a + (b-a)*u_random();
}
double exp_random(double const &l)
{
    return -l*log(u_random());
}
