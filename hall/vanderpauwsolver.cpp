#include "vanderpauwsolver.h"

#include <math.h>
#include <algorithm>

double VanDerPauwSolver::eval(double R12, double R23, double Rs)
{
    return exp(-M_PI*R12/Rs) + exp(-M_PI*R23/Rs) - 1;
}

double VanDerPauwSolver::solveSquareSample(double R)
{
    return M_PI*R/M_LN2;
}

std::pair<double, double> VanDerPauwSolver::solve(double R12, double R23, double eps, int iterations)
{
    if (!isfinite(R12) || !isfinite(R23) || R12 < 0. || R23 < 0. || !isfinite(eps))
        return std::make_pair<double, double>(NAN, NAN);

    if (R12 > R23)
        std::swap(R12, R23);

    double Rbottom(solveSquareSample(R12)), Rtop(solveSquareSample(R23));

    double dR, Rs;
    do {
        Rs = (Rtop + Rbottom) / 2.;

        if (eval(R12, R23, Rs) > 0.) {
            Rtop = Rs;
        } else
            Rbottom = Rs;

        dR = Rtop - Rbottom;
        if (dR <= eps || iterations <= 0)
            break;

        --iterations;
    } while(true);

    return std::make_pair<double, double>(Rs, dR);
}

