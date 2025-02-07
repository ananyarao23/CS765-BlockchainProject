#include <random>
using namespace std;

int generateExponential(double Ttx)
{
    std::random_device rd;
    std::mt19937 gen(rd());
    std::exponential_distribution<double> dist(1.0 / Ttx); // lambda = 1 / mean

    return (int)dist(gen);
}

double sampleUniform(double a, double b)
{
    // Create a random device and generator
    std::random_device rd;
    std::mt19937 gen(rd()); // Mersenne Twister RNG

    // Define the uniform distribution in range [a, b]
    std::uniform_real_distribution<double> dist(a, b);

    // Generate and return a random sample
    return dist(gen);
}


vector<int> randomIndices(int x, int n)
{
    static random_device rd;
    static mt19937 gen(rd());
    uniform_int_distribution<int> dist(0, n - 1);

    vector<int> indices;
    for (int i = 0; i < x; ++i)
    {
        indices.push_back(dist(gen));
    }

    return indices;
}
