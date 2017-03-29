/*
Copyright 2011-2017 Frederic Langlet
Licensed under the Apache License, Version 2.0 (the "License");
you may not use this file except in compliance with the License.
you may obtain a copy of the License at

                http://www.apache.org/licenses/LICENSE-2.0

Unless required by applicable law or agreed to in writing, software
distributed under the License is distributed on an "AS IS" BASIS,
WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
See the License for the specific language governing permissions and
limitations under the License.
*/

#ifndef _TextCodec_
#define _TextCodec_

#include "../Function.hpp"

namespace kanzi {
   class DictEntry {
   public:
       int32 _hash; // full hash
       int _pos; // position in text
       int16 _idx; // index in dictionary
       int16 _length; // length in text
       const byte* _buf; // data

       DictEntry();

       DictEntry(const byte buf[], int pos, int hash, int idx, int length);

       ~DictEntry() {}
   };

   // Simple one-pass text codec. Uses a default (small) static dictionary
   // or potentially larger custom one. Generates a dynamic dictionary.
   // Encoding: tokenize text into words. If word is in dictionary, emit escape
   // and word index (varint encode -> max 2 bytes). Otherwise, emit
   // word and add entry in dictionary with word position and length.
   // Decoding: If symbol is an escape, read word index (varint decode).
   // If current word is not in dictionary, add new entry. Otherwise,
   // emit current symbol.
   class TextCodec : public Function<byte> {
   public:
       static const int LOG_DICT_SIZE = 15;
       static const int DICTIONARY_SIZE = 1 << LOG_DICT_SIZE;
       static const int LOG_HASHES_SIZE = 24; // 16 MB
       static const byte DEFAULT_ESCAPE_TOKEN1 = '@';
       static const byte DEFAULT_ESCAPE_TOKEN2 = '^';

       TextCodec();

       TextCodec(byte dict[], int size, int logHashSize, int dictSize, byte delimiter1, byte delimiter2);

       virtual ~TextCodec()
       {
           delete[] _dictList;
           delete[] _dictMap;
       }

       bool forward(SliceArray<byte>& src, SliceArray<byte>& dst, int length);

       bool inverse(SliceArray<byte>& src, SliceArray<byte>& dst, int length);

       // Required encoding output buffer size
       // Space needed by destination buffer could be 3 x srcLength (if input data
       // is all delimiters). Limit to 1 x srcLength and ket the caller deal with
       // a failure when the output is not smaller than the input
       inline int getMaxEncodedLength(int srcLen) const { return srcLen; }

       inline static bool isText(byte val) { return TEXT_CHARS[val & 0xFF]; }

       inline static bool isLowerCase(byte val) { return (val >= 'a') && (val <= 'z'); }

       inline static bool isUpperCase(byte val) { return (val >= 'A') && (val <= 'Z'); }

       inline static bool isDelimiter(byte val) { return DELIMITER_CHARS[val & 0xFF]; }

   private:
       static const int HASH1 = 200002979;
       static const int HASH2 = 50004239;

       static bool* initDelimiterChars();
       static const bool* DELIMITER_CHARS;
       static bool* initTextChars();
       static const bool* TEXT_CHARS;

       static SliceArray<byte> unpackDictionary(const byte dict[], int dictSize);

       static bool sameWords(const byte src[], byte dst[], int length);

       // Default dictionary
       static const byte DICT_EN_1024[];

       // Static dictionary of 1024 entries.
       static DictEntry STATIC_DICTIONARY[1024];
       static int createDictionary(SliceArray<byte> words, DictEntry dict[], int maxWords, int startWord);
       static const int STATIC_DICT_WORDS;

       DictEntry** _dictMap;
       DictEntry* _dictList;
       byte _escape1;
       byte _escape2;
       byte _escapes[2];
       int _staticDictSize;
       int _logHashSize;
       int32 _hashMask;

       int emit(byte src[], byte dst[], const int srcEnd, const int dstEnd);
       int emit1(byte src[], byte dst[], const int srcEnd, const int dstEnd);
       int emit2(byte src[], byte dst[], const int srcEnd, const int dstEnd);
   };

   inline DictEntry::DictEntry()
   {
       _buf = nullptr;
       _pos = -1;
       _hash = 0;
       _idx = int16(0);
       _length = int16(0);
   }

   inline DictEntry::DictEntry(const byte buf[], int pos, int32 hash, int idx, int length)
   {
       _buf = buf;
       _pos = pos;
       _hash = hash;
       _idx = int16(idx);
       _length = int16(length);
   }
}
#endif