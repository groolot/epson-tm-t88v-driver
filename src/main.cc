#include "config.h"
#include <iostream>

int main ()
{
  std::cerr << PACKAGE_STRING
            << std::endl
            << "Rapport de bugs à : "
            << PACKAGE_BUGREPORT
            << std::endl << std::endl;

  std::cout << "Ceci est le programme nommé "
            << PACKAGE_NAME
            << std::endl;
  return 0;
}
