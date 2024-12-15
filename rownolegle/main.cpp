#pragma once
#include <iostream>
#include <Windows.h>
#include <vector>
#include <tuple>
#include <string>
#include <fstream>
#include <random>
#include <thread>
#include <chrono>

static void processSubsetDopasowanie(double** populacja, double** wielomiany, double* dopasowanie, int startIdx, int endIdx, int S) {
	for (int k = startIdx; k < endIdx; k++) {
		double wartosc = 0;
		double suma = 0;
		for (int i = 0; i < S; i++) {
			suma = 0;
			for (int j = 0; j < S; j++) {
				suma = (suma + wielomiany[i][j]) * populacja[k][i];
			}
			wartosc += suma;
		}
		dopasowanie[k] = wartosc;
	}
}

static void crossoverAndMutation(double** staraPopulacja, double** populacjaNowa, int N, int S, int startIdx, int endIdx, double T) {
	int m = 0, n = 0, r = 0;
	int i = startIdx, j = 0;
	double p = 0;

	std::random_device rnd;
	std::mt19937 gen(rnd());
	std::uniform_int_distribution<> dis(0, N - 1);
	std::uniform_int_distribution<> di2(0, S - 2);
	std::uniform_real_distribution<> d1r(0.0, 1.0);
	std::uniform_real_distribution<> d2r(-1.0, 1.0);

	while (i < endIdx) // krzyzowanie
	{
		m = dis(gen);
		do {
			n = dis(gen);
		} while (n == m);

		r = di2(gen);

		// krzyzowanie zgodnie z regula
		j = 0;
		while (j < r) {
			populacjaNowa[i][j] = staraPopulacja[m][j];
			j++;
		}
		while (j < S) {
			populacjaNowa[i][j] = staraPopulacja[n][j];
			j++;
		}

		i++;
	}

	// Mutacja
	while (i < endIdx) {
		j = 0;
		while (j < S) {
			p = d1r(gen);
			if (p < T) {
				populacjaNowa[i][j] = d2r(gen);
			}
			j++;
		}
		i++;
	}
}

static double** odczytPliku(std::string path, const int S)
{
	int i = 0, j = 0;
	double** wielomiany = new double* [S];
	for (i = 0; i < S; i++)
	{
		wielomiany[i] = new double[S];
	}

	std::ifstream file(path);
	if (!file) {
		std::cerr << "Error opening file!" << std::endl;
		return nullptr;
	}

	double value;
	i = 0;
	j = 0;
	while (file >> value)
	{
		wielomiany[j][i] = value;
		i++;
		if (i == S) { j++; i = 0; }
	}
	file.close();

	return wielomiany;
}

static double** createZeroPopulation(const int S, int N)
{
	std::random_device rnd;
	std::mt19937 gen(rnd());
	std::uniform_real_distribution<> dis(-1.0, 1.0);


	int i = 0, j = 0;
	double** populacjaZero = new double* [N];
	for (i = 0; i < N; i++)
	{
		populacjaZero[i] = new double[S];
		j = 0;
		while (j < S)
		{
			populacjaZero[i][j] = dis(gen);
			j++;
		}

	}

	return populacjaZero;
}

static void erase(double** something, const int S)
{
	for (int i = 0; i < S; ++i) {
		delete[] something[i];
	}
	delete something;
}

static double* dopasowanie(double** populacja, double** wielomiany, const int N, const int S,const int num)
{
	double* dopasowanie = new double[N];
	if (num < 2){
		double wartosc = 0;
		double suma = 0;
		for (int k = 0; k < N; k++){	
			wartosc = 0;
			for (int i = 0; i < S; i++){	
				suma = 0;
				for (int j = 0; j < S; j++){
					suma = (suma + wielomiany[i][j]) * populacja[k][i];
				}
				wartosc += suma;
			}
			dopasowanie[k] = wartosc;
		}
	}
	else
	{
		int rowsPerThread = N / num;
		std::vector<std::thread> threads;
		for (int i = 0; i < num; ++i) {
			int startIdx = i * rowsPerThread;
			int endIdx = (i == num - 1) ? N : (i + 1) * rowsPerThread;
			threads.push_back(std::thread(processSubsetDopasowanie, populacja, wielomiany, dopasowanie, startIdx, endIdx, S));
		}
		for (auto& t : threads) {
			t.join();
		}
	}
	return dopasowanie;
}

static double** generatePopulation(double** staraPopulacja, const int N, const int S, const double T, const int num)
{
	double** populacjaNowa = new double* [N];

	for (int i = 0; i < N; ++i) {
		populacjaNowa[i] = new double[S];
	}
	
	if (num < 2)
	{
		std::random_device rnd;
		std::mt19937 gen(rnd());
		std::uniform_int_distribution<> dis(0, N - 1);
		std::uniform_int_distribution<> di2(0, S - 2);
		std::uniform_real_distribution<> d1r(0.0, 1.0);
		std::uniform_real_distribution<> d2r(-1.0, 1.0);

		int m = 0, n = 0, r = 0;
		int i = 0, j = 0;
		double p = 0;

		while (i < N) // krzyzowanie
		{
			m = dis(gen);
			do
			{
				n = dis(gen);
			} while (n == m);

			r = di2(gen);

			// krzyzowanie wedlug reguly
			j = 0;
			while (j < r)
			{
				populacjaNowa[i][j] = staraPopulacja[m][j];
				j++;
			}
			while (j < S)
			{
				populacjaNowa[i][j] = staraPopulacja[n][j];
				j++;
			}

			i++;
		}

		// mutacje
		i = 0;

		while (i < N)
		{
			j = 0;
			while (j < S)
			{
				p = d1r(gen);
				if (p < T)
				{
					populacjaNowa[i][j] = d2r(gen);
				}

				j++;
			}

			i++;
		}
	}
	else
	{
		int rowsPerThread = N / num;
		std::vector<std::thread> threads;

		for (int i = 0; i < num; ++i) {
			int startIdx = i * rowsPerThread;
			int endIdx = (i == num - 1) ? N : (i + 1) * rowsPerThread;
			threads.push_back(std::thread(crossoverAndMutation, staraPopulacja, populacjaNowa, N, S, startIdx, endIdx, T));
		}
		for (auto& t : threads) {
			t.join();
		}
	}
	return populacjaNowa;
}

static double** optimizePopulation(double** olderPopulation, double** lastPopulation, double* dopasowanieOld, double* dopasowanieLast, const int N)
{
	std::vector<std::tuple<double, int, bool>> combinedFitness;

	for (int i = 0; i < N; ++i) {
		combinedFitness.push_back(std::make_tuple(dopasowanieOld[i], i, true));
	}

	for (int i = 0; i < N; ++i) {
		combinedFitness.push_back(std::make_tuple(dopasowanieLast[i], i, false));
	}

	std::sort(combinedFitness.begin(), combinedFitness.end(),
		[](const std::tuple<double, int, bool>& a, const std::tuple<double, int, bool>& b) {
			return std::get<0>(a) < std::get<0>(b);
		});

	double** optimizedPopulation = new double* [N];
	for (int i = 0; i < N; ++i) {
		double fitness = std::get<0>(combinedFitness[i]);
		int index = std::get<1>(combinedFitness[i]);
		bool isOld = std::get<2>(combinedFitness[i]);

		if (isOld) {
			optimizedPopulation[i] = olderPopulation[index];
		}
		else {
			optimizedPopulation[i] = lastPopulation[index];
		}
	}
	return optimizedPopulation;
}

static void writeDopasowanie(double* dopasowanie, const int N)
{
	for (int i = 0; i < N; i++)
	{
		printf("%.3f ", dopasowanie[i]);
	}
	std::cout << std::endl;
}

int main()
{
	double T = 0.2;
	int max_k = 40;//1024; // number of iterations
	const int S = 1024; //number of polynomials
	int k = 0; // algorithm iteration
	int i = 0; // iterator
	int N = 500; // number of individuals
	const std::string pathToFile = "C:\\Users\\smate\\Desktop\\computeFile.txt";
	int j = 0;
	int g = 10;

	double** wielomiany = odczytPliku(pathToFile, S);
	double** populacjaZero = createZeroPopulation(S, N);
	std::vector<double**> populacje;
	std::vector<double*> dopasowania;

	k = 0;
	populacje.push_back(populacjaZero);
	dopasowania.push_back(dopasowanie(populacje[k], wielomiany, N, S, g));
/*
	std::cout << "Full array:" << std::endl;
	for (int i = 0; i < 5; ++i) {
		for (int j = 0; j < 5; ++j) {
			std::cout << dopasowania[0][j] << " ";
		}
		std::cout << std::endl;
	}
	*/
	k = 1;
	auto start = std::chrono::high_resolution_clock::now();
	std::vector<std::thread> threads;
	// end of initialization

	while (k < max_k)
	{
		populacje.push_back(generatePopulation(populacje[k-1], N, S, T, g));// krok 3

		dopasowania.push_back(dopasowanie(populacje[k], wielomiany, N, S, g));// krok 4

		populacje[k] = optimizePopulation(populacje[k-1], populacje[k], dopasowania[k-1], dopasowania[k], N);// krok 5
		k++; // krok 6
		//writeDopasowanie(dopasowania[k - 1], N);
	}
	auto end = std::chrono::high_resolution_clock::now();
	std::chrono::duration<double> duration = end - start;
	std::cout << "Execution time: " << duration.count() << " s" << std::endl;
	printf("Najlepsze dopasowania:\n");
	writeDopasowanie(dopasowania[k - 1], N);

	erase(wielomiany, S);
	erase(populacjaZero, N);
	return 0;
}