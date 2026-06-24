#include "pricer/black_scholes.h"
#include "pricer/binomial.h"
#include "pricer/monte_carlo.h"
#include "pricer/option.h"

#include <iostream>
#include <iomanip>
#include <memory>
#include <chrono>

using namespace pricer;

void printResults(const std::string& method, const Option& option)
{
    std::cout << "\n========== " << method << " ==========\n";

    auto start = std::chrono::high_resolution_clock::now();

    double price = option.price();
    double delta = option.delta();
    double gamma = option.gamma();
    double theta = option.theta();
    double vega = option.vega();
    double rho = option.rho();

    auto end = std::chrono::high_resolution_clock::now();

    std::cout << std::fixed << std::setprecision(6);

    std::cout << "Price : " << price << "\n";
    std::cout << "Delta : " << delta << "\n";
    std::cout << "Gamma : " << gamma << "\n";
    std::cout << "Theta : " << theta << "\n";
    std::cout << "Vega  : " << vega << "\n";
    std::cout << "Rho   : " << rho << "\n";

    std::cout << "Execution Time : "
              << std::chrono::duration_cast<std::chrono::microseconds>(
                     end - start)
                     .count()
              << " us\n";

    std::cout << "===============================\n";
}

void runDemo()
{
    EuropeanOption euro_call(
        OptionType::Call,
        100.0,
        1.0,
        100.0,
        0.05,
        0.20,
        0.02
    );

    AmericanOption amer_put(
        OptionType::Put,
        100.0,
        1.0,
        100.0,
        0.05,
        0.20,
        0.02
    );

    auto bs_engine  = makeBlackScholesPricingEngine();
    auto mc_engine  = makeMonteCarloEngine(100000,252,true,0);
    auto bin_engine = makeBinomialTreeEngine(1000);

    std::cout << "\nEUROPEAN CALL OPTION\n";

    euro_call.setPricingEngine(bs_engine);
    printResults("Black-Scholes", euro_call);

    euro_call.setPricingEngine(mc_engine);
    printResults("Monte Carlo", euro_call);

    euro_call.setPricingEngine(bin_engine);
    printResults("Binomial Tree", euro_call);

    std::cout << "\nAMERICAN PUT OPTION\n";

    amer_put.setPricingEngine(mc_engine);
    printResults("Monte Carlo", amer_put);

    amer_put.setPricingEngine(bin_engine);
    printResults("Binomial Tree", amer_put);

    std::cout << "\nSpot Sensitivity Analysis\n";

    for(double spot : {90.0,95.0,100.0,105.0,110.0})
    {
        euro_call.setSpot(spot);
        euro_call.setPricingEngine(bs_engine);

        std::cout
            << "Spot = "
            << std::setw(6)
            << spot
            << " | Price = "
            << std::setw(10)
            << euro_call.price()
            << " | Delta = "
            << euro_call.delta()
            << "\n";
    }
}

void runInteractive()
{
    int typeChoice;
    int styleChoice;
    int methodChoice;

    double strike;
    double expiry;
    double spot;
    double rate;
    double volatility;
    double dividend;

    std::cout << "\nOption Type\n";
    std::cout << "1. Call\n";
    std::cout << "2. Put\n";
    std::cin >> typeChoice;

    std::cout << "\nOption Style\n";
    std::cout << "1. European\n";
    std::cout << "2. American\n";
    std::cin >> styleChoice;

    std::cout << "\nSpot Price: ";
    std::cin >> spot;

    std::cout << "Strike Price: ";
    std::cin >> strike;

    std::cout << "Time To Expiry (years): ";
    std::cin >> expiry;

    std::cout << "Risk Free Rate (0.05 = 5%): ";
    std::cin >> rate;

    std::cout << "Volatility (0.20 = 20%): ";
    std::cin >> volatility;

    std::cout << "Dividend Yield: ";
    std::cin >> dividend;

    OptionType type =
        (typeChoice == 1)
            ? OptionType::Call
            : OptionType::Put;

    std::unique_ptr<Option> option;

    if(styleChoice == 1)
    {
        option = std::make_unique<EuropeanOption>(
            type,
            strike,
            expiry,
            spot,
            rate,
            volatility,
            dividend
        );
    }
    else
    {
        option = std::make_unique<AmericanOption>(
            type,
            strike,
            expiry,
            spot,
            rate,
            volatility,
            dividend
        );
    }

    std::cout << "\nPricing Method\n";
    std::cout << "1. Black-Scholes\n";
    std::cout << "2. Monte Carlo\n";
    std::cout << "3. Binomial Tree\n";
    std::cin >> methodChoice;

    if(methodChoice == 1)
    {
        option->setPricingEngine(
            makeBlackScholesPricingEngine()
        );

        printResults("Black-Scholes", *option);
    }
    else if(methodChoice == 2)
    {
        size_t paths;

        std::cout << "Number of Paths: ";
        std::cin >> paths;

        option->setPricingEngine(
            makeMonteCarloEngine(paths)
        );

        printResults("Monte Carlo", *option);
    }
    else if(methodChoice == 3)
    {
        size_t steps;

        std::cout << "Number of Steps: ";
        std::cin >> steps;

        option->setPricingEngine(
            makeBinomialTreeEngine(steps)
        );

        printResults("Binomial Tree", *option);
    }
}

int main()
{
    try
    {
        while(true)
        {
            int choice;

            std::cout << "\n=====================================\n";
            std::cout << "      OPTION PRICING ENGINE\n";
            std::cout << "=====================================\n";
            std::cout << "1. Demo Mode\n";
            std::cout << "2. Interactive Pricing\n";
            std::cout << "3. Exit\n";
            std::cout << "Choice: ";

            std::cin >> choice;

            if(choice == 1)
            {
                runDemo();
            }
            else if(choice == 2)
            {
                runInteractive();
            }
            else if(choice == 3)
            {
                break;
            }
            else
            {
                std::cout << "Invalid Choice\n";
            }
        }
    }
    catch(const std::exception& e)
    {
        std::cerr << "\nError: "
                  << e.what()
                  << "\n";
    }

    return 0;
}