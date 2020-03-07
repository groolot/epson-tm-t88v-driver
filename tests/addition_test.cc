#include "addition_test.h"
#include "arithmetique.h"
#ifdef HAVE_CONFIG_H
  #include "config.h"
#endif

#include <limits>

// Enregistrer la classe de test dans le registre de la suite
CPPUNIT_TEST_SUITE_REGISTRATION(additionTest);

void additionTest::setUp() {}

void additionTest::tearDown() {}

void additionTest::addition_normal()
{
  operandeA = 1;
  operandeB = 2;
  CPPUNIT_ASSERT_EQUAL(static_cast<long int>(3),
                       static_cast<long int>(arithmetique::addition(operandeA, operandeB))
    );
  operandeB = -2;
  CPPUNIT_ASSERT_EQUAL(static_cast<long int>(-1),
                       static_cast<long int>(arithmetique::addition(operandeA, operandeB))
    );
}

void additionTest::addition_max()
{
  operandeA = std::numeric_limits<int>::max();
  operandeB = 1;
  CPPUNIT_ASSERT_GREATER(static_cast<long int>(operandeA),
                         static_cast<long int>(arithmetique::addition(operandeA, operandeB))
    );
}

void additionTest::addition_min()
{
  operandeA = std::numeric_limits<int>::lowest();
  operandeB = -1;
  CPPUNIT_ASSERT_LESS(static_cast<long int>(operandeA),
                      static_cast<long int>(arithmetique::addition(operandeA, operandeB))
    );
}

void additionTest::addition_zero()
{
  operandeA = operandeB = 0;
  CPPUNIT_ASSERT_EQUAL(static_cast<long int>(0),
                       static_cast<long int>(arithmetique::addition(operandeA, operandeB))
    );
  operandeA = 1;
  operandeB = -1;
  CPPUNIT_ASSERT_EQUAL(static_cast<long int>(0),
                       static_cast<long int>(arithmetique::addition(operandeA, operandeB))
    );
}
