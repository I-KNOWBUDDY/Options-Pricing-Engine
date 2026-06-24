#include "pricer/monte_carlo.h"
#include <cmath>
#include <thread>
#include <future>
#include <numeric>
#include <random>
#include <utility>
#include <iostream>
namespace pricer {

MonteCarloEngine::MonteCarloEngine(size_t num_paths, size_t num_steps,
                                 bool use_antithetic, size_t num_threads)
    : num_paths_(num_paths)
    , num_steps_(num_steps)
    , use_antithetic_(use_antithetic)
    , num_threads_(num_threads)
    , last_mean_(0.0)
    , last_stderr_(0.0) {

    if (num_paths_ == 0) {
        throw std::invalid_argument("Number of Monte Carlo paths must be positive");
    }
    if (num_steps_ == 0) {
        throw std::invalid_argument("Number of Monte Carlo steps must be positive");
    }

    if (num_threads_ == 0) {
        num_threads_ = std::thread::hardware_concurrency();
    }
    if (num_threads_ == 0) {
        num_threads_ = 1;
    }
    if (num_threads_ > num_paths_) {
        num_threads_ = num_paths_;
    }
}

double MonteCarloEngine::calculate(const Option& option) const {
    // Determine thread count and distribute paths evenly
    size_t thread_count = num_threads_;
    if (thread_count == 0) {
        thread_count = std::thread::hardware_concurrency();
    }
    if (thread_count == 0) {
        thread_count = 1;
    }
    if (thread_count > num_paths_) {
        thread_count = num_paths_;
    }

    const size_t base_paths = num_paths_ / thread_count;
    const size_t remainder = num_paths_ % thread_count;

    std::vector<std::future<std::pair<double, double>>> futures;
    futures.reserve(thread_count);

    for (size_t i = 0; i < thread_count; ++i) {
        const size_t num_paths = base_paths + (i < remainder ? 1 : 0);
        futures.push_back(
            std::async(std::launch::async,
                      &MonteCarloEngine::simulateBatch,
                      this,
                      std::ref(option),
                      static_cast<unsigned int>(i),
                      num_paths)
        );
    }

    // Collect results
    double sum_payoffs = 0.0;
    double sum_squared_payoffs = 0.0;
    size_t total_paths = 0;

    for (auto& future : futures) {
        auto result = future.get();
        sum_payoffs += result.first;
        sum_squared_payoffs += result.second;
    }

    for (size_t i = 0; i < thread_count; ++i) {
        total_paths += base_paths + (i < remainder ? 1 : 0);
    }

    if (total_paths == 0) {
        throw std::runtime_error("Monte Carlo engine has zero total paths");
    }

    const double mean = sum_payoffs / static_cast<double>(total_paths);
    const double variance = (sum_squared_payoffs / static_cast<double>(total_paths) - mean * mean);
    const double standard_error = std::sqrt(variance / static_cast<double>(total_paths));

    // Store statistics for confidence interval
    last_mean_ = mean;
    last_stderr_ = standard_error;

    // Discount to present value
    return mean * std::exp(-option.getRate() * option.getExpiry());
}



std::vector<double> MonteCarloEngine::generatePath(
    const Option& option,
    std::mt19937& rng,
    const bool antithetic) const {

    std::normal_distribution<double> normal(0.0, 1.0);

    const double S = option.getSpot();
    const double r = option.getRate();
    const double q = option.getDividend();
    const double sigma = option.getVolatility();
    const double T = option.getExpiry();

    const double dt = T / num_steps_;
    const double drift = (r - q - 0.5 * sigma * sigma) * dt;
    const double vol = sigma * std::sqrt(dt);

    std::vector<double> path(num_steps_ + 1);
    path[0] = S;

    for (size_t i = 0; i < num_steps_; ++i) {
        double z = normal(rng);
        if (antithetic) z = -z;  // Antithetic variate

        path[i + 1] = path[i] * std::exp(drift + vol * z);
    }

    return path;
}

double MonteCarloEngine::generateTerminalPrice(
    const Option& option,
    std::mt19937& rng,
    const bool antithetic) const {

    std::normal_distribution<double> normal(0.0, 1.0);
    double z = normal(rng);
    if (antithetic) z = -z;

    const double S = option.getSpot();
    const double r = option.getRate();
    const double q = option.getDividend();
    const double sigma = option.getVolatility();
    const double T = option.getExpiry();

    const double drift = (r - q - 0.5 * sigma * sigma) * T;
    const double vol = sigma * std::sqrt(T);

    return S * std::exp(drift + vol * z);
}

double MonteCarloEngine::calculatePayoff(
    const Option& option,
    double final_price) {

    double K = option.getStrike();

    if (option.getType() == OptionType::Call) {
        return std::max(final_price - K, 0.0);
    } else {
        return std::max(K - final_price, 0.0);
    }
}

std::pair<double, double> MonteCarloEngine::simulateBatch(
    const Option& option,
    unsigned int seed,
    size_t num_paths) const {

    std::mt19937 rng(seed);
    double sum_payoffs = 0.0;
    double sum_squared_payoffs = 0.0;

    const double S = option.getSpot();
    const double r = option.getRate();
    const double q = option.getDividend();
    const double sigma = option.getVolatility();
    const double T = option.getExpiry();

    const double drift = (r - q - 0.5 * sigma * sigma) * T;
    const double vol = sigma * std::sqrt(T);

    std::normal_distribution<double> normal(0.0, 1.0);

    for (size_t i = 0; i < num_paths; ++i) {
        double z = normal(rng);
        
        // Generate standard payoff
        double final_price = S * std::exp(drift + vol * z);
        double payoff = calculatePayoff(option, final_price);

        if (use_antithetic_) {
            // Generate antithetic payoff (use negated value of the SAME z)
            double anti_final_price = S * std::exp(drift + vol * (-z));
            double anti_payoff = calculatePayoff(option, anti_final_price);

            // Average the payoffs
            payoff = (payoff + anti_payoff) / 2.0;
        }

        sum_payoffs += payoff;
        sum_squared_payoffs += payoff * payoff;
    }

    return {sum_payoffs, sum_squared_payoffs};
}

std::pair<double, double> MonteCarloEngine::getConfidenceInterval(const Option& option) const {
    // 95% confidence interval (1.96 standard errors)
    constexpr double z_score = 1.96;
    double margin = z_score * last_stderr_;

    // Discount factor
    const double discount = std::exp(-option.getRate() * option.getExpiry());

    // Apply discounting to the mean and margin
    double discounted_mean = last_mean_ * discount;
    double discounted_margin = margin * discount;

    return {discounted_mean - discounted_margin, discounted_mean + discounted_margin};
}

double MonteCarloEngine::calculateDelta(const Option& option) const {
    // Use finite difference approximation
    const double h = 0.01 * option.getSpot();

    // Calculate up price
    double spot = option.getSpot();
    const_cast<Option&>(option).setSpot(spot + h);
    double up_price = calculate(option);

    // Calculate down price
    const_cast<Option&>(option).setSpot(spot - h);
    double down_price = calculate(option);

    // Reset spot price
    const_cast<Option&>(option).setSpot(spot);

    return (up_price - down_price) / (2.0 * h);
}

double MonteCarloEngine::calculateGamma(const Option& option) const {
    const double h = 0.01 * option.getSpot();
    double spot = option.getSpot();

    // Calculate up price
    const_cast<Option&>(option).setSpot(spot + h);
    double up_price = calculate(option);

    // Calculate center price
    const_cast<Option&>(option).setSpot(spot);
    double center_price = calculate(option);

    // Calculate down price
    const_cast<Option&>(option).setSpot(spot - h);
    double down_price = calculate(option);

    // Reset spot price
    const_cast<Option&>(option).setSpot(spot);

    return (up_price - 2.0 * center_price + down_price) / (h * h);
}

double MonteCarloEngine::calculateTheta(const Option& option) const {
    const double h = 1.0 / 365.0;  // One day
    double expiry = option.getExpiry();

    // Backup the original expiry value
    const_cast<Option&>(option).setExpiry(expiry + h);
    double price_plus_h = calculate(option);

    const_cast<Option&>(option).setExpiry(expiry - h);
    double price_minus_h = calculate(option);

    // Restore the original expiry value
    const_cast<Option&>(option).setExpiry(expiry);

    // Central difference formula scaled to daily Theta
    return -(price_plus_h - price_minus_h) / (2.0 * h * 365.0);
}

double MonteCarloEngine::calculateVega(const Option& option) const {
    const double h = 0.0001;
    double vol = option.getVolatility();

    const_cast<Option&>(option).setVolatility(vol + h);
    double up_price = calculate(option);

    const_cast<Option&>(option).setVolatility(vol - h);
    double down_price = calculate(option);

    const_cast<Option&>(option).setVolatility(vol);

    // Scaled by 100 for vega per 1% change
    return (up_price - down_price) / (2.0 * h * 100.0);
}

double MonteCarloEngine::calculateRho(const Option& option) const {
    const double h = 0.0001;
    double rate = option.getRate();

    const_cast<Option&>(option).setRate(rate + h);
    double up_price = calculate(option);

    const_cast<Option&>(option).setRate(rate - h);
    double down_price = calculate(option);

    const_cast<Option&>(option).setRate(rate);

    // Scaled by 100 for rho per 1% change
    return (up_price - down_price) / (2.0 * h * 100.0);
}

} // namespace pricer