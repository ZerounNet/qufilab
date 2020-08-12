/*
 *  @ Quant
 *  @ Anton Normelius, 2020.
 *
 *  Statistics calculations.
 *
 */

#include <iostream>
#include <vector>
#include <string>
#include <limits>
#include <numeric>
#include <pybind11/pybind11.h>
#include <pybind11/numpy.h>

#include "_stat.h"
#include "_trend.h"
#include "util.h"

namespace py = pybind11;

/*
 * Standard Deviation
 *
 * Calculates the standard deviation. 
 * Using normalization by default.
*/

py::array_t<double> std_calc(const py::array_t<double> prices,
         const int period, const bool normalize) {

    py::buffer_info prices_buf = prices.request();
    double *prices_ptr = (double *) prices_buf.ptr;
    const int size = prices_buf.shape[0];

    py::array_t<double> sma = sma_calc(prices, period);
    double *sma_ptr = (double *) sma.request().ptr;

    py::array_t<double> std = py::array_t<double>(prices_buf.size);
    double *std_ptr = (double *) std.request().ptr;
    init_nan(std_ptr, size);

    for (int ii = 0; ii < size - period+ 1; ii++) {
        double temp = 0;

        for (int idx = ii; idx < period+ ii; idx++) {
            temp += pow((prices_ptr[idx] - sma_ptr[ii+period-1]), 2);
        }

        if (normalize == true) {
            temp /= (period - 1);
        }

        else {
            temp /= (period);
        }
  
        temp = sqrt(temp);
        std_ptr[ii+period-1] = temp;
    }

    return std;
}

/*
 * Variance
 *
 * Calculates the variance. 
*/
py::array_t<double> var_calc(const py::array_t<double> prices,
         const int period, const bool normalize) {

    py::buffer_info prices_buf = prices.request();
    double *prices_ptr = (double *) prices_buf.ptr;
    const int size = prices_buf.shape[0];

    py::array_t<double> sma = sma_calc(prices, period);
    double *sma_ptr = (double *) sma.request().ptr;

    py::array_t<double> var = py::array_t<double>(prices_buf.size);
    double *var_ptr = (double *) var.request().ptr;
    init_nan(var_ptr, size);

    int adjust_nan = 0;
    for (int idx = 0; idx < size; ++idx) {
        if (std::isnan(prices_ptr[idx])) {
            ++adjust_nan;
        }

        else {
            break;
        }
    }

    for (int ii = 0 + adjust_nan; ii < size - period + 1; ii++) {
        double temp = 0;

        for (int idx = ii; idx < period+ ii; idx++) {
            temp += pow((prices_ptr[idx] - sma_ptr[ii+period-1]), 2);
        }

        if (normalize == true) {
            temp /= (period- 1);
        }

        else {
            temp /= (period);
        }
  
        //temp = sqrt(temp);
        var_ptr[ii+period-1] = temp;
    }

    return var;
}

/*
 * Covariance
 *
 * Calculates the covariance between one price array and another.. 
*/
py::array_t<double> cov_calc(const py::array_t<double> prices, const py::array_t<double> market,
         const int period, const bool normalize) {

    py::buffer_info prices_buf = prices.request();
    double *prices_ptr = (double *) prices_buf.ptr;
    const int size = prices_buf.shape[0];
    double *market_ptr = (double *) market.request().ptr;

    py::array_t<double> sma = sma_calc(prices, period);
    py::array_t<double> sma_market = sma_calc(market, period);
    double *sma_ptr = (double *) sma.request().ptr;
    double *sma_market_ptr = (double *) sma_market.request().ptr;

    py::array_t<double> cov = py::array_t<double>(prices_buf.size);
    double *cov_ptr = (double *) cov.request().ptr;
    init_nan(cov_ptr, size);
    
    int adjust_nan = 0;
    for (int idx = 0; idx < size; ++idx) {
        if (std::isnan(prices_ptr[idx])) {
            ++adjust_nan;
        }

        else {
            break;
        }
    }

    for (int ii = 0 + adjust_nan; ii < size - period+ 1; ii++) {
        double temp = 0;

        for (int idx = ii; idx < period + ii; idx++) {
            temp += ((prices_ptr[idx] - sma_ptr[ii+period-1]) * 
                (market_ptr[idx] - sma_market_ptr[ii+period-1]));
        }

        if (normalize == true) {
            temp /= (period - 1);
        }

        else {
            temp /= (period);
        }
  
        cov_ptr[ii+period-1] = temp;
    }

    return cov;
}

/*
 * Beta
 *
 * Calculates the beta coefficient for a price array.
 * 
 * @param prices (py::array_t<double>): Array with prices.
 * @param market (py::array_t<double>): Array with market prices that beta is calculated from.
 * @param period (int): Number of periods.
 * @param normalize (bool): Specify whether to normalize the variance of the market.
 *
 * Observe that the first array is the array that beta will be calcualted for, and the second
 * is the market that serves as the comparison.
 *
 */
py::array_t<double> beta_calc(const py::array_t<double> prices, const py::array_t<double> market,
        const int period, const bool var_normalize) {

    py::buffer_info prices_buf = prices.request();
    const int size = prices_buf.shape[0];
    
    py::array_t<double> prices_pct = pct_change_calc(prices, 1);
    py::array_t<double> market_pct = pct_change_calc(market, 1);

    // Calculate the variance for the market.
    py::array_t<double> var = var_calc(market_pct, period, var_normalize);
    double *var_ptr = (double *) var.request().ptr;

    // Calculate covariance between price and market.
    py::array_t<double> cov = cov_calc(prices_pct, market_pct, period, false);
    double *cov_ptr = (double *) cov.request().ptr;
    
    py::array_t<double> beta = py::array_t<double>(prices_buf.size);
    double *beta_ptr = (double *) beta.request().ptr;
    init_nan(beta_ptr, size);

    for (int idx = period; idx < size; ++idx) {
        beta_ptr[idx] = cov_ptr[idx] / var_ptr[idx];
    }

    return beta;
}

py::array_t<double> pct_change_calc(const py::array_t<double> prices, const int period) {
    py::buffer_info prices_buf = prices.request();
    const int size = prices_buf.shape[0];
    double *prices_ptr = (double *) prices_buf.ptr;

    py::array_t<double> pct_change = py::array_t<double>(prices_buf.size);
    double *pct_change_ptr = (double *) pct_change.request().ptr;
    init_nan(pct_change_ptr, size);

    for (int idx = period; idx < size; ++idx) {
        pct_change_ptr[idx] = ((prices_ptr[idx] - prices_ptr[idx-period]) / 
            prices_ptr[idx-period]) * 100;
    }

    return pct_change;
}

PYBIND11_MODULE(_stat, m) {
    m.def("std_calc", &std_calc, "Standard Deviation");
    m.def("var_calc", &var_calc, "Variance");
    m.def("cov_calc", &cov_calc, "Covariance");
    m.def("beta_calc", &beta_calc, "Beta");
    m.def("pct_change_calc", &pct_change_calc, "Percentage change");
}
