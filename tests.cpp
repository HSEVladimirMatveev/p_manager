#include <cassert>
#include "mycrypto.hpp"
#include "kuznechik.hpp"
#include <iostream>



void unit_test_hex_to_bytes(const char text [], const char hex_text [])
{
  ByteBlock result = hex_to_bytes(hex_text);
  bool flag = true;
  for (int j = 0; j < result.size(); j++)
  {
    if (text[j] != result[j])
    {
      std::cout << "text[j]" << text[j] << std::endl;
      std::cout << "hex_text[j]" << result[j] << std::endl;
      flag = false;
    }
  }
  assert(flag == true);
}

void unit_test_hex_representation(const char text[], unsigned size, const char hex_text[])
{
  ByteBlock bb(reinterpret_cast<const BYTE *> (text), size);
  bool flag = true;
  std::string result = hex_representation(bb);
  if (result != hex_text)
  {
    flag = false;
  }
  assert(flag == true);
}

void Tests_hex_representation()
{
  const char text1[] = "def";
  const char text2[] = "";
  const char text3[] = "@";
  const char text4[] = "FALSE";
  const char hex_text1[] = "646566";
  const char hex_text2[] = "";
  const char hex_text3[] = "40";
  const char hex_text4[] = "46414C5346";
  unsigned size1 = 3;
  unsigned size2 = 0;
  unsigned size3 = 1;
  unsigned size4 = 5;
  unit_test_hex_representation(text1, size1, hex_text1); 
  std::cout << "Hex_representation test 1/4 ok." << std::endl;
  unit_test_hex_representation(text2, size2, hex_text2);
  std::cout << "Hex_representation test 2/4 ok." << std::endl;
  unit_test_hex_representation(text3, size3, hex_text3);
  std::cout << "Hex_representation test 3/4 ok." << std::endl;
  unit_test_hex_representation(text4, size4, hex_text4);
  std::cout << "All hex_representation tests 4/4 OK!" << std::endl; 
}

void Tests_hex_to_bytes()
{
  const char text1[] = "def";
  const char text2[] = "";
  const char text3[] = "@";
  const char text4[] = "FALSE";
  const char text5[] = "TRUEYX";
  const char hex_text1[] = "646566";
  const char hex_text2[] = "";
  const char hex_text3[] = "40";
  const char hex_text4[] = "46414C5346";
  const char hex_text5[] = "46414C53469";
  unit_test_hex_to_bytes(text1, hex_text1);
  std::cout << "Hex_to_bytes test 1/5 ok." << std::endl;
  unit_test_hex_to_bytes(text2, hex_text2);
  std::cout << "Hex_to_bytes test 2/5 ok." << std::endl;
  unit_test_hex_to_bytes(text3, hex_text3);
  std::cout << "Hex_to_bytes test 3/5 ok." << std::endl;
  unit_test_hex_to_bytes(text4, hex_text4);
  std::cout << "Hex_to_bytes test 4/5 ok." << std::endl;
  unit_test_hex_to_bytes(text5, hex_text5);
  std::cout << "All hex_to_bytes tests 5/5 OK!" << std::endl;
}
/*
int main()
{
  Tests_hex_to_bytes();
  //Tests_hex_representation();
  //ByteBlock bb(reinterpret_cast<const BYTE *> ("FALSE"), 5);
  //BYTE b = 'A';
  //char res = to_hex_literal(b);
  return 1;
}
*/