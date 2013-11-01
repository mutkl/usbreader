/* -*- mode: c++ -*- */
#ifndef IWORD
#define IWORD

#include <iostream>
#include <string>
#include <vector>

using namespace std;

static int NUM_BITS_PER_WORD = 16; // n of bits per word
static const int LARGE_NUMBER=5000;
                                                      

class Word {

  string word_;

public:

  // default constructor
  Word()
    {
      word_="";
    }


  // constructor
  Word(string word)
    {

      if( word.size() < NUM_BITS_PER_WORD )
	word_ = word;
      else
	word_ = string(word,0,NUM_BITS_PER_WORD);
      if( word.size() != NUM_BITS_PER_WORD ){
	clog << " warning: a word has length " << word.size() << " instead of " << NUM_BITS_PER_WORD << " word: " << word << " stored as: " << word_ << endl;
	exit(0);
      }

    }


  //! destructor
  virtual ~Word(){};

  virtual void dump (ostream & a_out         = cout,
		     const string & a_title  = "",
		     const string & a_indent = ""){
    {
      string indent;
      if (! a_indent.empty ()) indent = a_indent;
      if (! a_title.empty ())
	{
	  a_out << indent << a_title << endl;
	}

      a_out << word_ << endl;

      return;
    }
  }


  void set( string word ) {
    word_ = string(word,0,NUM_BITS_PER_WORD);
    if( word.size() != NUM_BITS_PER_WORD ){
      clog << " warning: a word has length " << word.size() << " instead of " << NUM_BITS_PER_WORD << " word: " << word << " stored as: " << word_ << endl;
      exit(0);
    }
  }
  
  void set_word( string word ) {
    word_ = string(word,0,NUM_BITS_PER_WORD);
    if( word.size() != NUM_BITS_PER_WORD ){
      clog << " warning: a word has length " << word.size() << " instead of " << NUM_BITS_PER_WORD << " word: " << word << " stored as: " << word_ << endl;
      exit(0);
    }
  }
  
  string word(void)
  {
    return word_;
  }


};

#endif

