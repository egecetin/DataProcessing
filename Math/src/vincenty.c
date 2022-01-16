#include "vincenty.h"

double Vincenty_Distance( const double& latitude_01, const double& longitude_01,
                          const double& latitude_02, const double& longitude_02,
                          const double& a,
                          const double& b )
{
    double sin_lambda, cos_lambda, sin_alpha;
    double c1, c2, c3, C;

    // Flattening
    const double f = (a-b)/a;

    // tan U1
    const double tan_U1 = (1-f) * std::tan(latitude_01);
    const double tan_U2 = (1-f) * std::tan(latitude_02);

    // Longitudinal Distance
    const double cos_U1 = 1 / std::sqrt(1 + tan_U1 * tan_U1);
    const double cos_U2 = 1 / std::sqrt(1 + tan_U2 * tan_U2);
    const double sin_U1 = tan_U1 * cos_U1;
    const double sin_U2 = tan_U2 * cos_U2;

    // Iterate until complete
    const double L = longitude_02 - longitude_01;
    double lambda = L;
    double diff=1, sigma;
    double cos_alpha_sq, cos_2sigma_m;
    double cos_sigma, sin_sigma;

    while( std::fabs(diff) < 0.00001 ){

        sin_lambda = std::sin( lambda );
        cos_lambda = std::cos( lambda );

        c1 = (cos_U2 * sin_lambda)*(cos_U2 * sin_lambda);
        c2 = (cos_U1 * sin_U2);
        c3 = (sin_U1 * cos_U2 * cos_lambda);

        //  sin sigma
        sin_sigma = std::sqrt( c1 + ( c2 - c3 )*( c2 - c3 ) );

        // cos sigma
        cos_sigma = sin_U1 * sin_U2 + cos_U1 * cos_U2 * cos_lambda;

        // sigma
        sigma = std::atan2( sin_sigma, cos_sigma );

        // sin alpha
        sin_alpha = (cos_U1 * cos_U2 * sin_lambda)/(sin_sigma);

        // cos^2 alpha
        cos_alpha_sq = 1 - (sin_alpha*sin_alpha);

        // cos^2 2sigmam
        cos_2sigma_m = cos_sigma - (2 * sin_U1 * sin_U2)/(cos_alpha_sq);

        // C
        C = (f/16.0) * cos_alpha_sq * (4 + f * (4 - 3 * cos_alpha_sq));

        // Update Lambda
        diff = lambda;
        lambda = L + (1-C) * f * sin_alpha * (sigma + C * sin_sigma * ( cos_2sigma_m + C * cos_sigma * (-1 + 2 * cos_2sigma_m*cos_2sigma_m)));
        diff = lambda - diff;
    }

    // U2
    double u_sq = cos_alpha_sq  * (a*a - b*b)/(b*b);

    // Compute A, B
    double A = 1 + (u_sq/16384) * (4096 + u_sq * (-768 + u_sq * (320 - 175 * u_sq))); // Can be simple with Helmert's Expansion

    double B = (u_sq / 1024) * (256 + u_sq * (-128 + u_sq * (-128 + u_sq * (74 - 47 * u_sq)))); // Can be simple with Helmert's Expansion

    // Sigma
    double cos_2sigma_m_sq = cos_2sigma_m * cos_2sigma_m;
    double delta_sigma = B * sin_sigma * ( cos_2sigma_m + (B/4.0) * (cos_sigma * (-1 * 2 * cos_2sigma_m_sq ) - (B/6.0) * cos_2sigma_m * (-3 + 4 * sin_sigma*sin_sigma) * (-3 + 4 * cos_2sigma_m_sq)));

    // Distance
    return b * A * (sigma - delta_sigma);
}