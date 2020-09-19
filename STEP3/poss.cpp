// poisson_distribution
#include <iostream>
#include <chrono>
#include <random>

using namespace std;
int main()
{
  const int nrolls = 19; // number of experiments
  const int nstars = 10;   // maximum number of stars to distribute

  unsigned seed = std::chrono::system_clock::now().time_since_epoch().count();
  std::default_random_engine generator (seed);
  std::poisson_distribution<int> distribution(6);

  int p[10]={};

  for (int i=0; i<nrolls; ++i) {
    int number = distribution(generator);
    if(number > 8)
    	cout << number << endl;
    //if (number<10) ++p[number];
  }
/*
  std::cout << "poisson_distribution (mean=6):" << std::endl;
  for (int i=0; i<10; ++i)
    std::cout << i << ": " << std::string(p[i]*nstars/nrolls,'*') << std::endl;
*/
  return 0;
}

//http://www.cplusplus.com/reference/random/poisson_distribution/
