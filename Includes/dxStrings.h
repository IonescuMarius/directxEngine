/****************************************************************************************
*																						*
* dxStrings.h -- string Object 															*
*																						*
* Copyright (c) Ionescu Marius. All rights reserved(2011).								*
*																						*
*																						*
* Author: Marius Ionescu <yonescu_marius@yahoo.com>										*
* Copyright (c) 2009-2011, Informaticha Studio											*
*																						*
* Licensed under the Microsoft Permissive License (Ms-PL), Version 1.1 (the "License")  *
* you may not use this file except in compliance with the License.						*
* You may obtain a copy of the license at												*
*																						*
*   http://directx.codeplex.com/license													*
*																						*
* Unless required by applicable law or agreed to in writing, software					*
* distributed under the License is distributed on an "AS IS" BASIS,						*
* WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.				*
* See the License for the specific language governing permissions and					*
* limitations under the License.														*
*																						*
****************************************************************************************/


#ifndef _DXSTRING_H_
#define _DXSTRING_H_

#include <string>
using namespace std;

class Strings {
   char *p;
   int size;
 public:
   Strings(char *str);
   Strings(void);
   Strings(const Strings &obj);           // Copy constructor
   ~Strings(void) {delete [] p;}

   friend ostream &operator<<(ostream &stream, Strings &obj);
   friend istream &operator>>(istream &stream, Strings &obj);

   Strings operator=(Strings &obj);       // assign a Strings object
   Strings operator=(char *s);            // assign a quoted string
   Strings operator+(Strings &obj);       // concatenate a Strings object
   Strings operator+(char *s);            // concatenate a quoted string
   friend Strings operator+(char *s, Strings &obj);
            /* concatenates a quoted string with a Strings object */

   Strings operator-(Strings &obj);       // subtract a Strings object
   Strings operator-(char *s);            // subtract a quoted string

 /* relational operators between Strings objects. Note that the operators could
    just as easily return bool, rather than int */

   int operator==(Strings &obj) {return !strcmp(p, obj.p);}
   int operator!=(Strings &obj) {return strcmp(p, obj.p);}
   int operator<(Strings &obj) {return strcmp(p, obj.p) < 0;}
   int operator>(Strings &obj) {return strcmp(p, obj.p) > 0;}
   int operator<=(Strings &obj) {return strcmp(p, obj.p) <= 0;}
   int operator>=(Strings &obj) {return strcmp(p, obj.p) >= 0;}

 /* relational operators between Strings object and a quoted character string.
    Note that the operators could just as easily return bool, rather than int */

   int operator==(char *s) {return !strcmp(p, s);}
   int operator!=(char *s) {return strcmp(p, s);}
   int operator<(char *s) {return strcmp(p, s) < 0;}
   int operator>(char *s) {return strcmp(p, s) > 0;}
   int operator<=(char *s) {return strcmp(p, s) <= 0;}
   int operator>=(char *s) {return strcmp(p, s) >= 0;}

   int strsize(void) {return strlen(p);}      // return string size
   void makestr(char *s) {strcpy(s, p);}  // make quoted string from Strings object

   operator char *(void) {return p;}          // conversion to char
 };

 #endif
