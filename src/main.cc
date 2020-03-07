#include "config.h"
#include "arithmetique.h"

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
            << std::endl << std::endl;

  std::cout << "123 + 9 = " << arithmetique::addition(123, 9) << std::endl;
  std::cout << "123 - 9 = " << arithmetique::soustraction(123, 9) << std::endl;
  std::cout << "123 * 9 = " << arithmetique::multiplication(123, 9) << std::endl;
  std::cout << "123 / 9 = " << arithmetique::division(123, 9) << std::endl;
  return 0;
}
