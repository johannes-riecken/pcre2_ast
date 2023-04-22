# JSON parser using callouts from regex

This is a proof of concept of parsing JSON by calling predefined functions while the regex matcher is in progress. 
This technique has been [demonstrated in Perl](https://www.perlmonks.org/?node_id=995856) before.
By using the PCRE2 library, I show that it's possible to use this technique in a wide variety of languages.

## Debugging with PCRE2

pcre2test has -ac and -AC options. You can write `haystack\=callout_extra` to
get backtracking info. The current problem is that the failing match causes the
callout `push_array` to be called when the array variable on the stack is
invalid. pcre2test doesn't reveal how this happens or why it only happens with
"[5\n", but not with "[5".
