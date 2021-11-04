#include "../include/perlin_noise.h"



using namespace ZED;



double PerlinNoise::Noise(int X, int Y)
{
    //Twister random( ((m_Seed * X) << 16) | ((m_Seed * Y) & 0x00FF));
    //return random.Rand();

    int primes[10][3] = {
  { 995615039, 600173719, 701464987 },
  { 831731269, 162318869, 136250887 },
  { 174329291, 946737083, 245679977 },
  { 362489573, 795918041, 350777237 },
  { 457025711, 880830799, 909678923 },
  { 787070341, 177340217, 593320781 },
  { 405493717, 291031019, 391950901 },
  { 458904767, 676625681, 424452397 },
  { 531736441, 939683957, 810651871 },
  { 997169939, 842027887, 423882827 }
    };
    const int i = 0;

    int n = X + Y * 57;
    n = (n << 13) ^ n;
    int a = primes[i][0], b = primes[i][1], c = primes[i][2];
    int t = (n * (n * n * a + b) + c) & 0x7fffffff;
    return 1.0 - (double)(t) / 1073741824.0;
}



double PerlinNoise::SmoothedNoise(int X, int Y)
{
    double corners = Noise(X - 1, Y - 1) + Noise(X + 1, Y - 1) + Noise(X - 1, Y + 1) + Noise(X + 1, Y + 1);
    double sides   = Noise(X - 1, Y) + Noise(X + 1, Y) + Noise(X, Y - 1) + Noise(X, Y + 1);
    double center  = Noise(X, Y);
    return corners/16.0 + sides/8.0 + center/4.0;
}



double PerlinNoise::Interpolate(double A, double B, double X)
{
    //Cosine interpolation
    //double X_interpol = (1 - cos(X * 3.1415927)) * 0.5;

    //Improved Perlin Noise from NVIDIA
    double X3 = X * X * X;
    double X_interpol = 6.0 * X3 * X * X  -  15.0 * X3*X  +  10.0 * X3;

    return  A * (1 - X_interpol) + B * X_interpol;
}



double PerlinNoise::Noise2D(double X, double Y)
{
    int integer_X = (int)std::floor(X);
    int integer_Y = (int)std::floor(Y);

    double fractional_X = X - integer_X;
    double fractional_Y = Y - integer_Y;

    double v1 = SmoothedNoise(integer_X, integer_Y);
    double v2 = SmoothedNoise(integer_X + 1, integer_Y);
    double v3 = SmoothedNoise(integer_X, integer_Y + 1);
    double v4 = SmoothedNoise(integer_X + 1, integer_Y + 1);

    double i1 = Interpolate(v1, v2, fractional_X);
    double i2 = Interpolate(v3, v4, fractional_X);
    return Interpolate(i1, i2, fractional_Y);
}