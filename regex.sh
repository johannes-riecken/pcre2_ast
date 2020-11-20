#!/bin/sh
echo 123|pcre2grep '(*NO_AUTO_POSSESS)(*NO_DOTSTAR_ANCHOR)(*NO_START_OPT)\A(?&digit_list)\z(?(DEFINE)(?<digit_list>(?C"|create
")((?&digit)(?C"|snoc
"))*)(?<digit>(\d)(?C"|last_paren
")))'
