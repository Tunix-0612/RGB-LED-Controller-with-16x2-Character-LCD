#include "TunixSelfTest.h"

TunixSelfTest::TunixSelfTest() { }

// --- Arduino check free RAM, return by byt
int TunixSelfTest::getFreeRAM() 
{
  extern int __heap_start, *__brkval; 
  int v; 
  return (int) &v - (__brkval == 0 ? (int) &__heap_start : (int) __brkval); 
}

// --- Arduino RAM test
bool TunixSelfTest::runRAMTest() 
{
  // RAM start and end adresses for ATMEGA328P
  // Start Adress 0x0100 (Internal SRAM)
  // End Adress RAMEND (0x08FF)
  noInterrupts(); // Disable Interrupts
  extern int __heap_start, *__brkval;
  
  // Adresleri doğrudan int* (pointer) tipinde tutalım
  int* free_memory_start = (__brkval == 0 ? &__heap_start : __brkval);
  
  // RAMEND işaretsiz değerini güvenli bir şekilde pointer'a cast ediyoruz
  int* free_memory_end = (int*)((uintptr_t)RAMEND - 32); // Preserve the upper 32 stack

  // Scan the Dynamical RAM area that has been defined
  // Artık ptr da free_memory_end de 'int*' tipinde olduğu için uyarı vermeyecek
  for (int* ptr = free_memory_start; ptr < free_memory_end; ptr++) 
  {
    int originalValue = *ptr; // Back-up the original value

    // Test 1: Write 0x55 and read (01010101 01010101)
    *ptr = 0x5555;
    if (*ptr != 0x5555) { interrupts(); return true; } // Faulty RAM

    // Test 2: Write 0xAA and read (10101010 10101010)
    *ptr = (int)0xAAAA; // 0xAAAA'yı signed int sınırında tutmak için cast edebilirsin
    if (*ptr != (int)0xAAAA) { interrupts(); return true; } // Faulty RAM
    
    *ptr = originalValue; // Load the original value back
  }
  interrupts();
  return false; // All Registers are Healthy
}

byte TunixSelfTest::selfTest()
{
	if(runRAMTest()) return RAM_FAILURE_CODE;
		
	// Write device specified selft test checks
	return 0;
}