# JSON parser using callouts from regex

This is a proof of concept of parsing JSON by calling predefined functions while the regex matcher is in progress. 
This technique has been [demonstrated in Perl](https://www.perlmonks.org/?node_id=995856) before.
By using the PCRE2 library, I show that it's possible to use this technique in a wide variety of languages.
