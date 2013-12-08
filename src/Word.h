/* -*- mode: c++ -*- */
#ifndef WORD_H
#define WORD_H

#include <iostream>
#include <string>
#include <vector>
#include <stdlib.h> 


const static int NUM_BITS_PER_WORD  = 16; // n of bits per word
const static int LARGE_NUMBER = 5000;


class Word {
  
private:
  
  std::string word_;
  
public:
  Word();
  Word(std::string word);
  ~Word();
  
  void dump();
  
  void set( std::string word );  
  void set_word( std::string word );
  std::string word() { return word_; }


};

#endif

