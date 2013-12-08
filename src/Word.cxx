#include "Word.h"

// default constructor
Word::Word()
{
  word_="";
}


// constructor
Word::Word(std::string word)
{
  if( word.size() < NUM_BITS_PER_WORD )
    word_ = word;
  else
    word_ = std::string(word,0,NUM_BITS_PER_WORD);
  if( word.size() != NUM_BITS_PER_WORD ){
    std::cout << " warning: a word has length " << word.size() << " instead of " << NUM_BITS_PER_WORD << " word: " << word << " stored as: " << word_ << std::endl;
    exit(0);
  }
  
}

Word::~Word(){}


void Word::dump(){
  std::cout << word_ << std::endl;  
}


void Word::set( std::string word ) {
  word_ = std::string(word,0,NUM_BITS_PER_WORD);
  if( word.size() != NUM_BITS_PER_WORD ){
    std::cout << " warning: a word has length " << word.size() << " instead of " << NUM_BITS_PER_WORD << " word: " << word << " stored as: " << word_ << std::endl;
    exit(0);
  }
}

void Word::set_word( std::string word ) {
  word_ = std::string(word,0,NUM_BITS_PER_WORD);
  if( word.size() != NUM_BITS_PER_WORD ){
    std::cout << " warning: a word has length " << word.size() << " instead of " << NUM_BITS_PER_WORD << " word: " << word << " stored as: " << word_ << std::endl;
    exit(0);
  }
}
