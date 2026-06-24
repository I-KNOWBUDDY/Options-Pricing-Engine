#include "pricer/utils.h"
#include "pricer/black_scholes.h"
#include "pricer/option.h"
#include <gtest/gtest.h>
#include <vector>
#include <cmath>

TEST(UtilsTest, ImpliedVolatilityCall) {
    double spot = 100.0;
    double strike = 100.0;
    double time = 1.0;
    double rate = 0.05;
    double dividend = 0.0;
    double target_vol = 0.20;

    // First, compute standard BS price for target_vol
    auto engine = pricer::makeBlackScholesPricingEngine();
    pricer::EuropeanOption option(pricer::OptionType::Call, strike, time, spot, rate, target_vol, dividend);
    option.setPricingEngine(engine);
    double market_price = option.price();

    // Now calculate implied vol using market_price
    double iv = pricer::utils::impliedVolatility(market_price, spot, strike, rate, time, dividend, true);
    
    EXPECT_NEAR(iv, target_vol, 1e-4);
}

TEST(UtilsTest, ImpliedVolatilityPut) {
    double spot = 100.0;
    double strike = 95.0;
    double time = 0.5;
    double rate = 0.03;
    double dividend = 0.02;
    double target_vol = 0.25;

    // First, compute standard BS price for target_vol
    auto engine = pricer::makeBlackScholesPricingEngine();
    pricer::EuropeanOption option(pricer::OptionType::Put, strike, time, spot, rate, target_vol, dividend);
    option.setPricingEngine(engine);
    double market_price = option.price();

    // Now calculate implied vol using market_price
    double iv = pricer::utils::impliedVolatility(market_price, spot, strike, rate, time, dividend, false);
    
    EXPECT_NEAR(iv, target_vol, 1e-4);
}

TEST(UtilsTest, ImpliedVolatilityInvalid) {
    // Under intrinsic price should throw invalid_argument
    EXPECT_THROW({
        pricer::utils::impliedVolatility(0.01, 100.0, 90.0, 0.05, 1.0, 0.0, true);
    }, std::invalid_argument);

    // Negative market price should throw invalid_argument
    EXPECT_THROW({
        pricer::utils::impliedVolatility(-10.0, 100.0, 100.0, 0.05, 1.0, 0.0, true);
    }, std::invalid_argument);
}

TEST(UtilsTest, HistoricalVolatility) {
    // Standard daily prices
    std::vector<double> prices = {100.0, 101.0, 102.0, 100.0, 99.0, 101.0};
    double time_step = 1.0 / 252.0; // Daily step in years

    double vol = pricer::utils::historicalVolatility(prices, time_step);
    EXPECT_GT(vol, 0.0);

    // Flat prices should have 0 volatility
    std::vector<double> flat_prices = {100.0, 100.0, 100.0, 100.0};
    double flat_vol = pricer::utils::historicalVolatility(flat_prices, time_step);
    EXPECT_NEAR(flat_vol, 0.0, 1e-7);
}
