# Option Pricing Engine (C++20)

A quantitative finance project written in modern C++20 for pricing European and American options using multiple pricing methodologies.

The project implements:

* Black-Scholes Analytical Pricing
* Monte Carlo Simulation
* Binomial Tree Pricing (CRR Model)
* Greeks Calculation
* Volatility Utilities
* Performance Comparison Between Models

---

# Features

## Pricing Models

### Black-Scholes

Analytical solution for European options.

Calculates:

* Price
* Delta
* Gamma
* Theta
* Vega
* Rho

---

### Monte Carlo Simulation

Uses stochastic simulation of asset paths.

Features:

* Multi-threading
* Antithetic Variance Reduction
* Confidence Interval Estimation
* Greeks via Finite Differences

---

### Binomial Tree (CRR)

Discrete-time pricing model.

Supports:

* European Options
* American Options
* Early Exercise
* Richardson Extrapolation

---

# Option Types and Styles

## Option Types

Defined in `option.h`:

* `OptionType::Call`: Standard call options
* `OptionType::Put`: Standard put options

## Option Styles

Defined in `option.h`:

* `EuropeanOption`: Exercisable only at maturity
* `AmericanOption`: Exercisable at any time before maturity

## Mapping to Pricing Engines

* **Black-Scholes (****`BlackScholesPricingEngine`****)**
  Prices European call and put options.

* **Monte Carlo (****`MonteCarloEngine`****)**
  Prices European call and put options using optimized terminal price simulations.

* **Binomial Tree (****`BinomialTreeEngine`****)**
  Prices both European and American call and put options, including early exercise checks.

---

# Greeks Supported

| Greek | Description                  |
| ----- | ---------------------------- |
| Delta | Sensitivity to Spot Price    |
| Gamma | Sensitivity of Delta         |
| Theta | Time Decay                   |
| Vega  | Sensitivity to Volatility    |
| Rho   | Sensitivity to Interest Rate |

---

# Project Structure

```text
Options Pricing Engine
│
├── include/
│   └── pricer/
│       ├── option.h
│       ├── engine.h
│       ├── black_scholes.h
│       ├── monte_carlo.h
│       ├── binomial.h
│       └── utils.h
│
├── src/
│   ├── option.cpp
│   ├── black_scholes.cpp
│   ├── monte_carlo.cpp
│   ├── binomial.cpp
│   └── utils.cpp
│
├── tests/
│   ├── test_black_scholes.cpp
│   ├── test_monte_carlo.cpp
│   ├── test_binomial.cpp
│   └── test_main.cpp
│
├── examples/
│   └── basic_usage.cpp
│
├── main.cpp
│
└── README.md
```

---

# Build

### Main Application (Windows - MSYS2 / MinGW)

```powershell
g++ -std=c++20 main.cpp src/*.cpp -Iinclude -O2 -pthread -o options_pricer.exe
```

### Test Runner

To build the test suite executable (`test_runner.exe`):

```powershell
g++ -std=c++20 tests/*.cpp src/*.cpp -Iinclude -O2 -pthread -o test_runner.exe
```

---

# Run

### Run Main Application

```powershell
.\options_pricer.exe
```

### Run Tests

```powershell
.\tests\test_runner.exe
```

---

# Sample Output

```text
European Call Option

Black-Scholes:
Price = 9.2270
Delta = 0.5869
Gamma = 0.0189

Monte Carlo:
Price = 9.2154

Binomial Tree:
Price = 9.2311
```

---

# Validation

The implementation has been tested against:

* Put-Call Parity
* Black-Scholes Benchmarks
* Monte Carlo Convergence
* Binomial Convergence
* American Option Early Exercise Conditions
* Greeks Consistency Checks

---

# Notes

* `test_runner.exe` is a compiled test executable and should **not be committed** to version control.
* Add it to your `.gitignore` file:

```gitignore
*.exe
*.o
```

---

# Applications

This project demonstrates concepts used in:

* Quantitative Finance
* Derivatives Pricing
* Risk Management
* Computational Finance
* Algorithmic Trading

---

# Future Improvements

* Implied Volatility Solver
* Asian Options
* Barrier Options
* Volatility Surface Generation
* Market Data Integration
* Portfolio Risk Analytics

---

# Technologies Used

* C++20
* STL
* Object-Oriented Design
* Numerical Methods
* Multithreading (`std::thread`)
* Monte Carlo Simulation

---

# Author

**Prerak Jain**
