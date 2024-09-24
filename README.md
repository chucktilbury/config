# Config
A simple configuration file reader.

This implements a simple config file reader. The file format is very simple, but allows for a wide variety of data to be stored.

## Sections.
Sections in the file are optional and nestable. It defined by a name followed by curly braces. The name is the same format as a name in C, or the regular expression of ``[a-zA-Z_][a-z-A-Z_0-9]*``. Everything enclosed by the curly braces takes the form of ``name = value``. The value can be a quoted string of any length and extend across newlines. Otherwise, a vlaue is taken to be on a single line. Comments are introduced by a ``;`` character and extend to the end of the line.

All values that are defined in the config file are returned as strings. A layer of conversion routines may be added later to change values such as number to a native value.

Names are accessed by the section name and the defined name for example:
```
; this is a comment
section {
  something = blart
}
```
A call such as ``const char* str = get_config("section.something")`` sets the value of ``str`` to "blart". All values returned by get_config() are ``const char*`` and must not be changed by the caller.

Another example:
```
bacon {
  number = 1234
}
eggs {
  number = 8080
}
```
A call such as 
```
const char* str1 = get_config("bacon.number"); // returns "1234"
const char* str2 = get_config("eggs.number"); // returns "8080"
```

Another example:
```
beef=a patty
bacon {
  number = 1234
  eggs {
    number = 8080
  }
  longer=
"this is a long
string with
newlines in it"
}
```
A call such as 
```
const char* str0 = get_config("beef"); // returns "a patty"
const char* str1 = get_config("bacon.number"); // returns "1234"
const char* str2 = get_config("bacon.eggs.number"); // returns "8080"
const char* str3 = get_config("bacon.eggs"); // returns NULL
const char* str4 = get_config("bacon.eggs.toast"); // returns NULL
```

Note that all non-printable characters are ignored except for the ``\n`` character. All comments are treated as non-printable characters. 

## Implementation
The implementation is a simple flex and bison combo. There are no keywords. The data structure that is returned is a simple hash table that indexes simple strings. 

## The Future
In the future, I may add a command line capability and reading variables from the shell environment. 
* The command line stuff already exists and I have written it several times. Basically, I would simply need to integrate the output into the data structure. Note that a command line option is required to find the config file.
* The environment is trivial, but different implementations would be required for different operating systems, so I defer that until I actually need it.

  

