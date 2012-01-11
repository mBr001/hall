#ifndef VANDERPAUWSOLVER_H
#define VANDERPAUWSOLVER_H

#include <utility>

class VanDerPauwSolver
{
protected:
    static double eval(double R12, double R23, double Rs);

public:
    static double solveSquareSample(double R);
    static std::pair<double, double> solve(double R12, double R23, double eps = 0., int iterations = 4000);
};

#endif // VANDERPAUWSOLVER_H
