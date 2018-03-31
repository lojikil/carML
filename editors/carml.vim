" Vim syntax file
" Language: carML
" Maintainer: lojikil
" Latest Revision: 18 June 2016
" add this to your .vimrc:
" au BufRead,BufNewFile *.29 set filetype=xl29
" au BufRead,BufNewFile *.carml set filetype=xl29

if exists("b:current_syntax")
  finish
endif

syn keyword basicLanguageKeywords val def let letrec in when if then else
syn keyword basicLanguageKeywords do match case with of use record type poly
syn keyword basicLanguageKeywords and var for while import use open load
syn keyword xl29BlockCmd begin end 

" Integer with - + or nothing in front
syn match xl29Number '\s\d\+\s'
syn match xl29Number '\s[-+]\d\+\s'

" Floating point number with decimal no E or e 
syn match xl29Number '\s[-+]\d\+\.\d*\s'

" Floating point like number with E and no decimal point (+,-)
syn match xl29Number '[-+]\=\d[[:digit:]]*[eE][\-+]\=\d\+'
syn match xl29Number '\d[[:digit:]]*[eE][\-+]\=\d\+'

" Floating point like number with E and decimal point (+,-)
syn match xl29Number '[-+]\=\d[[:digit:]]*\.\d*[eE][\-+]\=\d\+'
syn match xl29Number '\d[[:digit:]]*\.\d*[eE][\-+]\=\d\+'

syn region xl29DescBlock start="{" end="}" fold transparent
syn region xl29DescBlock start="begin" end="end" fold transparent 
syn keyword xl29Todo contained TODO FIXME XXX NOTE
syn match xl29Comment "#.*$" contains=xl29Todo

syn keyword xl29Type int char float string bool deque
syn keyword xl29Ops add sum array println print ref div mod divide modulo sub subtract

let b:current_syntax = "xl29"

hi def link basicLanguageKeywords Statement
hi def link xl29Todo        Todo
hi def link xl29Comment     Comment
hi def link xl29BlockCmd    Statement
hi def link xl29Type        Type
hi def link xl29String      Constant
hi def link xl29Desc        PreProc
hi def link xl29Number      Constant
hi def link xl29Ops         Operator
