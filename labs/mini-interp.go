package main

/*
 * this is the result of two files being smooshed together, plus some
 * custom Golang:
 *
 * . labs/mini-sexpression.carml, for reading in carML/C SExpression output
 * . labs/mini-walk.carml, for actually executing code
 *
 * Currently, it actually is just an interpreter for carML itself, but that
 * should change eventually
 */

import (
    "fmt"
    "reflect"
    "strconv"
)

type SExpression interface {
	isSExpression()
}
type SExpression_Nil struct {
}
func (SExpression_Nil) isSExpression() {}
type SExpression_Atom struct {
	m_1 string
	m_2 int
}
func (SExpression_Atom) isSExpression() {}
type SExpression_String struct {
	m_1 string
	m_2 int
}
func (SExpression_String) isSExpression() {}
type SExpression_Int struct {
	m_1 string
	m_2 int
}
func (SExpression_Int) isSExpression() {}
type SExpression_Float struct {
	m_1 string
	m_2 int
}
func (SExpression_Float) isSExpression() {}
type SExpression_Char struct {
	m_1 string
	m_2 int
}
func (SExpression_Char) isSExpression() {}
type SExpression_Bool struct {
	m_1 bool
	m_2 int
}
func (SExpression_Bool) isSExpression() {}
type SExpression_List struct {
	m_1 []SExpression 
	m_2 int
}
func (SExpression_List) isSExpression() {}
type SExpression_Error struct {
	m_1 string
	m_2 int
}
func (SExpression_Error) isSExpression() {}
type SExpression_Null struct {
}
func (SExpression_Null) isSExpression() {}
type SExpression_EndList struct {
	m_1 int
}
func (SExpression_EndList) isSExpression() {}
type SExpression_EndArray struct {
	m_1 int
}
func (SExpression_EndArray) isSExpression() {}
type SExpression_EndFile struct {
}
func (SExpression_EndFile) isSExpression() {}

type Token struct {
	lexeme string
	lexeme_offset int
	lexeme_type int
}
func is_whitespace(ch byte) bool {
	switch ch {
		case ' ':
			return true
		case '\t':
			return true
		case '\n':
			return true
		case '\v':
			return true
		case '\r':
			return true
		case '\b':
			return true
		default:
			return false
	}

}
func take_while_white(src string, start int) int {
	var idx int = start
	var ch byte = src[start]
	if is_whitespace(ch) {
		for is_whitespace(ch) {
			ch = src[idx]
			idx = (idx + 1)
		}

		return idx - 1
	}

	return idx
}
func take_until_break(src string, start int) int {
	var idx int = start
	var ch byte = ' '
	if idx > len(src) {
		return idx
	} else {
		ch = src[idx]
		for (idx < len(src)) && (is_symbolic(ch)) {
			idx = (idx + 1)
			ch = src[idx]
		}

		return idx

	}

}
func next(src string, start_offset int) Token  {
	if start_offset >= len(src) {
		return Token{ "", start_offset, 98}
	}

	offset := take_while_white(src, start_offset)
	ch := src[offset]
	var idx int = offset
	var state int = 0
	switch {
		case ch == '(':
			return Token{ "(", offset, 1}
		case ch == ')':
			return Token{ ")", offset, 2}
		case ch == '[':
			return Token{ "[", offset, 3}
		case ch == ']':
			return Token{ "]", offset, 4}
		case (is_numeric(ch)):
			for idx < len(src) {
				switch state {
					case 0:
						switch {
							case ch == '.':
								state = 1
							case (is_numeric(ch)):
								state = 0
							case (is_symbolic(ch)):
								state = 2
							case (!is_symbolic(ch)):
								return Token{ string_slice(src, offset, idx), offset, 5}
							default:
								state = 0
						}

					case 1:
						switch {
							case ch == '.':
								state = 2
							case (is_numeric(ch)):
								state = 1
							case (is_symbolic(ch)):
								state = 2
							case (!is_symbolic(ch)):
								return Token{ string_slice(src, offset, idx), offset, 6}
							default:
								state = 1
						}

					case 2:
						if !is_symbolic(ch) {
							return Token{ string_slice(src, offset, idx), offset, 7}
						}

				}

				idx = (idx + 1)
				if idx < len(src) {
					ch = src[idx]
				}

			}


		case (is_symbolic(ch)):
			for (idx < len(src)) && (is_symbolic(ch)) {
				ch = src[idx]
				idx = (idx + 1)
			}

			return Token{ string_slice(src, offset, idx - 1), offset, 7}

		case ch == '"':
			idx = (idx + 1)
			if idx >= len(src) {
				return Token{ "malformed string", offset, 99}
			} else {
				ch = src[idx]
			}

			for (ch != '"') && (idx < len(src)) {
				switch ch {
					case '\\':
						idx = (idx + 2)
					default:
						idx = (idx + 1)
				}

				if idx < len(src) {
					ch = src[idx]
				}

			}

			return Token{ string_slice(src, offset, idx + 1), offset, 8}

		case ch == '#':
			var tmp string = ""
			var tidx int = 0
			idx = (idx + 1)
			if (idx + 1) >= len(src) {
				return Token{ "malformed character", offset, 99}
			}

			if src[idx] != '\\' {
				return Token{ "character must be followed by \\", offset, 99}
			}

			tidx = take_until_break(src, idx + 1)
			tmp = string_slice(src, idx + 1, tidx)
			switch {
				case (1 == len(tmp)):
					return Token{ tmp, offset + 2, 9}
				case tmp == "newline":
					return Token{ "\n", offset + 8, 9}
				case tmp == "tab":
					return Token{ "\t", offset + 4, 9}
				case tmp == "carriage":
					return Token{ "\r", offset + 9, 9}
				case tmp == "bell":
					return Token{ "\a", offset + 5, 9}
				case tmp == "vtab":
					return Token{ "\v", offset + 5, 9}
				case tmp == "backspace":
					return Token{ "\b", offset + 10, 9}
				case tmp == "backslash":
					return Token{ "\\", offset + 10, 9}
				default:
					return Token{ "incorrect named character", offset, 99}
			}


		default:
			return Token{ " ", 1, 99}
	}

	switch state {
		case 0:
			return Token{ string_slice(src, offset, idx), offset, 5}
		case 1:
			return Token{ string_slice(src, offset, idx), offset, 6}
		case 2:
			return Token{ string_slice(src, offset, idx), offset, 7}
	}

	return Token{ " ", 1, 99}
}
func is_numeric(ch byte) bool {
	switch {
		case ((ch >= '0') && (ch <= '9')):
			return true
		case ch == '.':
			return true
		default:
			return false
	}

}
func is_symbolic(ch byte) bool {
	switch {
		case ch == '(':
			return false
		case ch == ')':
			return false
		case ch == '[':
			return false
		case ch == ']':
			return false
		case ch == '"':
			return false
		case ch == '\'':
			return false
		case ch == '#':
			return false
		case (is_whitespace(ch)):
			return false
		default:
			return true
	}

}
func is_null_or_endp(obj SExpression ) bool {
	switch obj.(type) {
		case SExpression_Null:
			return true
		case SExpression_EndList:
			return true
		case SExpression_EndArray:
			return true
		case SExpression_EndFile:
			return true
		default:
			return false
	}

}
func shrink_array(src []SExpression , cap int, length int) []SExpression  {
	var ret []SExpression  = make([]SExpression, length)
	var idx int = 0
	for idx < length {
		ret[idx] = src[idx]
		idx = (idx + 1)
	}

	return ret
}
func read_list(src string, start int) SExpression  {
	var ret []SExpression  = make([]SExpression, 128)
	var ret_length int = 128
	var tmp SExpression = sexpression_read(src, start + 1)
	var idx int = 0
	var offset int = start
	for !is_null_or_endp(tmp) {
		if idx >= ret_length {
			return SExpression_Error{ "read_list array length overrun", start}
		}

		ret[idx] = tmp
		idx = (idx + 1)
		switch tmp := tmp.(type) {
			case SExpression_List:
				offset = tmp.m_2
			case SExpression_Atom:
				offset = (tmp.m_2 + len(tmp.m_1))
			case SExpression_String:
				offset = (tmp.m_2 + len(tmp.m_1))
			case SExpression_Int:
				offset = (tmp.m_2 + len(tmp.m_1))
			case SExpression_Float:
				offset = (tmp.m_2 + len(tmp.m_1))
			case SExpression_Bool:
                if tmp.m_1 {
				    offset = (tmp.m_2 + 4)
                } else {
                    offset = (tmp.m_2 + 5)
                }
			case SExpression_Char:
				offset = (tmp.m_2 + len(tmp.m_1))
			case SExpression_EndList:
				offset = (tmp.m_1 + 1)
			default:
				return tmp
		}

		tmp = sexpression_read(src, offset)
	}

	return SExpression_List{ shrink_array(ret, 128, idx), offset + 1}
}
func read_char(src string, offset int) SExpression  {
	tok := next(src, offset)
	if tok.lexeme_type == 9 {
		return SExpression_Char{ tok.lexeme, tok.lexeme_offset}
	} else {
		return SExpression_Error{ "expected character", offset}
	}

}
func read_atom(src string, offset int) SExpression  {
	tok := next(src, offset)
	if tok.lexeme_type == 7 {
		return SExpression_Atom{ tok.lexeme, tok.lexeme_offset}
	} else {
		return SExpression_Error{ "expected atom", offset}
	}

}
func read_string(src string, offset int) SExpression  {
	tok := next(src, offset)
	if tok.lexeme_type == 8 {
		return SExpression_String{ tok.lexeme, tok.lexeme_offset}
	} else {
		return SExpression_Error{ "expected character", offset}
	}

}
func string_slice(src string, start int, endpoint int) string {
	len := endpoint - start
	var res []byte  = make([]byte, len)
	var idx int = 0
	for idx < len {
		res[idx] = src[idx + start]
		idx = (idx + 1)
	}

	return string(res)
}
func read_number(src string, offset int) SExpression  {
	tok := next(src, offset)
	l16807 := tok.lexeme_type
	switch l16807 {
		case 5:
			return SExpression_Int{ tok.lexeme, tok.lexeme_offset}
		case 6:
			return SExpression_Float{ tok.lexeme, tok.lexeme_offset}
		default:
			return SExpression_Error{ "wanted to read a number, but other type returned", offset}
	}

}
func sexpression_read(src string, start int) SExpression  {
	tok := next(src, start)
	l282475249 := tok.lexeme_type
	switch l282475249 {
		case 1:
			return read_list(src, tok.lexeme_offset)
		case 2:
			return SExpression_EndList{ tok.lexeme_offset + 1}
		case 5:
			return SExpression_Int{ tok.lexeme, tok.lexeme_offset}
		case 6:
			return SExpression_Float{ tok.lexeme, tok.lexeme_offset}
		case 7:
			return SExpression_Atom{ tok.lexeme, tok.lexeme_offset}
		case 8:
			return SExpression_String{ tok.lexeme, tok.lexeme_offset}
		case 9:
			return SExpression_Char{ tok.lexeme, tok.lexeme_offset}
		case 98:
			return SExpression_EndFile{ }
		default:
			return SExpression_Error{ tok.lexeme, tok.lexeme_offset}
	}

}
func sexpression2char(src string) string {
	l1622650073 := src[0]
	switch l1622650073 {
		case '\n':
			return "#\\newline"
		case '\v':
			return "#\\vtab"
		case '\r':
			return "#\\carriage"
		case '\t':
			return "#\\tab"
		case '\b':
			return "#\\backspace"
		case '\a':
			return "#\\bell"
		case '\u0000':
			return "#\\nul"
		default:
			return "#\\" + src
	}

}
func sexpression2string(src SExpression ) string {
	var result []string
	var flattened []byte
	var idx int = 0
	var cidx int = 0
	var ridx int = 0
	var totallen int = 0
	switch src := src.(type) {
		case SExpression_Int:
			return src.m_1
		case SExpression_Float:
			return src.m_1
		case SExpression_Atom:
			return src.m_1
		case SExpression_String:
			flattened = (make([]byte, 2 + len(src.m_1)))
			idx = 1
			cidx = 1
			for cidx < ((len(src.m_1)) - 1) {
				flattened[cidx] = src.m_1[idx]
				idx = (idx + 1)
				cidx = (cidx + 1)
			}

			flattened[0] = '"'
			flattened[idx] = '"'
			return string(flattened)

		case SExpression_Char:
			return sexpression2char(src.m_1)
		case SExpression_List:
			result = (make([]string, len(src.m_1)))
			for idx < len(result) {
				result[idx] = sexpression2string(src.m_1[idx])
				totallen = (totallen + len(result[idx]))
				idx = (idx + 1)
			}

			totallen = (totallen + ((len(result)) + 2))
			flattened = (make([]byte, totallen))
			flattened[0] = '('
			idx = 1
			for idx < ((len(flattened)) - 1) {
				cidx = 0
				for cidx < len(result[ridx]) {
					flattened[idx] = result[ridx][cidx]
					idx = (idx + 1)
					cidx = (cidx + 1)
				}

				flattened[idx] = ' '
				idx = (idx + 1)
				ridx = (ridx + 1)
			}

			flattened[idx] = ')'
			return string(flattened)

	}
	return string(flattened)

}

func roundtrip(src string) {
    fmt.Printf("src:\n%s\n", src)
    g := sexpression_read(src, 0)
    fmt.Printf("g: %V\n", g)
    h := read_list(src, 0)
    fmt.Printf("h: %V\n", h)
    f := sexpression2string(g)
    fmt.Printf("f: %v\n", f)
    d := sexpression2string(h)
    fmt.Printf("d: %v\n", d)
}

func muwalk(src SExpression , env SExpression ) SExpression  {
    return SExpression_Nil {};
}
func sexpression_eqp(s0 SExpression , s1 SExpression ) bool {
	var idx int = 0
	var llen int = 0
	if reflect.TypeOf(s0) != reflect.TypeOf(s1) {
		return false
	}

	switch s0 := s0.(type) {
		case SExpression_Nil:
			return true
		case SExpression_EndFile:
			return true
		case SExpression_EndList:
			return true
		case SExpression_EndArray:
			return true
		case SExpression_Atom:
			return s0.m_1 == s1.(SExpression_Atom).m_1
		case SExpression_Int:
			return s0.m_1 == s1.(SExpression_Int).m_1
		case SExpression_Float:
			return s0.m_1 == s1.(SExpression_Float).m_1
		case SExpression_Char:
			return s0.m_1 == s1.(SExpression_Char).m_1
		case SExpression_Bool:
			return s0.m_1 == s1.(SExpression_Bool).m_1
		case SExpression_Error:
			return s0.m_1 == s1.(SExpression_Error).m_1
		case SExpression_String:
			return s0.m_1 == s1.(SExpression_String).m_1
		case SExpression_List:
			llen = len(s0.m_1)
			for idx < llen {
				if sexpression_eqp(s0.m_1[idx], s1.(SExpression_List).m_1[idx]) {
					idx = (idx + 1)

				} else {
					return false

				}

			}

			return true

	}
    return false
}
func assoc(src SExpression , dst SExpression ) SExpression  {
	var idx int = 0
	var llen int = len(dst.(SExpression_List).m_1)
	for idx < llen {
		if sexpression_eqp(first(dst.(SExpression_List).m_1[idx]), src) {
			return first(rest(dst.(SExpression_List).m_1[idx]))
		}

		idx = (idx + 1)
	}

	return SExpression_Nil{ }
}
func mem_assoc(needle SExpression , src SExpression ) SExpression  {
	var idx int = 0
	var llen int = len(needle.(SExpression_List).m_1)
	for idx < llen {
		if sexpression_eqp(first(needle.(SExpression_List).m_1[idx]), src) {
			return sexpression_boxbool(true)
		}

		idx = (idx + 1)
	}

	return sexpression_boxbool(false)
}
func cons(hd SExpression , dst SExpression ) SExpression  {
    return SExpression_Nil{ }
}
func first(hd SExpression ) SExpression  {
	switch hd := hd.(type) {
		case SExpression_List:
			return hd.m_1[0]

		default:
			return SExpression_Error{ "first can only be applied to lists", 0}
	}

}
func rest(l SExpression ) SExpression  {
	var res []SExpression  = make([]SExpression, (len(l.(SExpression_List).m_1)) - 1)
	var idx int = 1
	var llen int = len(l.(SExpression_List).m_1)
    fmt.Printf("idx: %d, llen: %d\n", idx, llen)
	for idx < llen {
        fmt.Printf("idx: %d, llen: %d\n", idx, llen)
		res[idx - 1] = l.(SExpression_List).m_1[idx]
        fmt.Printf("res[%d] = %v\n", idx - 1, res[idx - 1])
		idx = (idx + 1)
	}

	return SExpression_List{ res, 0}
}

func mapfn(f func (SExpression) SExpression, l SExpression , e SExpression ) SExpression  {
    return SExpression_Nil { }
}

func mapeval(l SExpression , e SExpression ) SExpression  {
	var res []SExpression  = make([]SExpression, len(l.(SExpression_List).m_1))
	var idx int = 0
	for idx != len(l.(SExpression_List).m_1) {
		res[idx] = mueval(l.(SExpression_List).m_1[idx], e)
		idx = (idx + 1)
	}

	return SExpression_List{ res, 0}
}
func sexpression_boxint(x int64) SExpression  {
	return SExpression_Int{ strconv.FormatInt(x, 10), 0}
}
func sexpression_boxbool(b bool) SExpression  {
	return SExpression_Bool{ b, 0}
}
func mueval(src SExpression , env SExpression ) SExpression  {
	switch src := src.(type) {
		case SExpression_List:
			var hd SExpression = src.m_1[0]
			switch hd.(type) {
                case SExpression_Atom:
                    switch hd.(SExpression_Atom).m_1 {
                        case "define":

                        case "define-value":

                        case "define-mutable-value":

                        case "if":

                        case "call":
                            tmp := muapply(src, env)
                            fmt.Printf("tmp: %v\n", tmp)
                            return tmp

                        default:
                            return first(rest(src))
                    }
                default:
                    return src

			}


		default:
			return src
	}
    return src
}
func muapply(src SExpression , env SExpression ) SExpression  {
	var lst SExpression = SExpression_Null{ }
	var hd SExpression = SExpression_Null{ }
    var tmp0 SExpression = SExpression_Null { }
    var tmp1 SExpression = SExpression_Null { }

    switch src.(type) {
        case SExpression_List:
            src = src.(SExpression_List)
        default:
            return src
    }

	lst = mapeval(rest(src), env)
	hd = first(lst)
    fmt.Printf("src: %v\n", src)
    fmt.Printf("lst: %v\n", lst)
    fmt.Printf("hd: %v\n", hd)
    fmt.Println("here on 735?")
	switch hd.(SExpression_String).m_1 {
		case "\"+\"":
            fmt.Println("here on 737")
            tmp0 = first(rest(lst))
            tmp1 = first(rest(rest(lst)))
            fmt.Printf("tmp0: %T%v\n", tmp0, tmp0)
            fmt.Printf("tmp1: %T%v\n", tmp1, tmp1)
            switch tmp0.(type) {
                case SExpression_Int:
                    fmt.Println("here on 741")
                    switch tmp1.(type) {
                        case SExpression_Int:
                            fmt.Println("here on 743")
                            n, err := strconv.ParseInt(tmp0.(SExpression_Int).m_1, 10, 64)
                            if err != nil {
                                return SExpression_Error{"integer parse error 0", 0}
                            }
                            m, err := strconv.ParseInt(tmp1.(SExpression_Int).m_1, 10, 64)
                            if err != nil {
                                return SExpression_Error{"integer parse error 1", 0}
                            }
                            fmt.Printf("here: %d\n", n + m)
                            return sexpression_boxint(n + m)
                        //case SExpression_Float:
                        default:
                            return SExpression_Error{"mismatched types 0", 0}
                    }
                case SExpression_Float:
                    return SExpression_Error{"mismatched types 1", 0}
                default:
                    return SExpression_Error{"mismatched types 2", 0}
            }
        default:
            fmt.Printf("weird, but hd.m_1 is %v\n", hd.(SExpression_String).m_1)
            fmt.Printf("weird, but hd.m_1 is %v\n", hd.(SExpression_String).m_1 == "+")
        /*
		case "-":
			return sexpression_boxint(first(lst).m_1 - first(rest(lst)).m_1)

		case "*":
			return sexpression_boxint(first(lst).m_1 * first(rest(lst)).m_1)

		case "/":
			return sexpression_boxint(first(lst).m_1 / first(rest(lst)).m_1)
		case "<":
			return sexpression_boxbool(first(lst).m_1 < first(rest(lst)).m_1)

		case "<=":
			return sexpression_boxbool(first(lst).m_1 <= first(rest(lst)).m_1)

		case ">":
			return sexpression_boxbool(first(lst).m_1 > first(rest(lst)).m_1)

		case ">=":
			return sexpression_boxbool(first(lst).m_1 >= first(rest(lst)).m_1)

		case "<>":
			return sexpression_boxbool(first(lst).m_1 != first(rest(lst)).m_1)
        */
	}
    return SExpression_Null { }
}

func main() {
    src := `(define main 

    (returns (type integer))
    (begin
        (call (identifier printf) (string "this is a test\n"))
        (call (identifier printf) (string "test test test\n"))
        (integer 0)))`
    src1 := `(define main (returns (type integer)) (begin (integer 0)))`
    src2 := `(call "+" (integer 10) (call "+" (integer 20) (integer 3)))`
    roundtrip(src)
    roundtrip(src1)
    roundtrip(src2)
    v := sexpression_read(src2, 0)
    fmt.Printf("%v\n", mueval(v, SExpression_Null { }))
}
