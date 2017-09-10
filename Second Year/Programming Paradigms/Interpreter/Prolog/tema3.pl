valnum(X):- char_type(X, alnum), char_type(X, ascii).
vother(X):- member(X, [';','<','+','-','*','(',')','{','}']).
validc(X):- valnum(X) ; vother(X) ;  X == '='.

lparseq(['='|L],'==',L).
lparseq([X|L],'=',[X|L]):-dif(X,'=').
lparseq([],'=',[]).

lparsealn([X|L],L2,R,L3):- valnum(X), lparsealn(L, [X|L2], R, L3).
lparsealn([X|L],L2,R,[X|L]):- \+valnum(X), reverse(L2, L3), atom_chars(R, L3).
lparsealn([],L2,R,[]):- reverse(L2, L3), atom_chars(R, L3).

lparse2(['='|L],L2,L3):- lparseq(L,R,L4), lparse2(L4,[R|L2],L3).
lparse2([X|L],L2,L3):- valnum(X),lparsealn(L,[X],R,L4), lparse2(L4,[R|L2],L3).
lparse2([X|L],L2,L3):- vother(X), lparse2(L,[X|L2],L3).
lparse2([X|L],L2,L3):- \+validc(X), lparse2(L,L2,L3).
lparse2([],L2,L3):- reverse(L2,L3).

lparse(S, L):- atom_chars(S, L2), lparse2(L2,[],L),!.

/* Predicat folosit pentru delimitarea unei expresii */
%parseExpr(L, R, Rest)
parseExpr([;|T], [], T):- !.
parseExpr([')'|T], [], T):- !.
parseExpr([*|T], [*|R], Rest):- parseExpr(T, R, Rest), !.
parseExpr([+|T], [+|R], Rest):- parseExpr(T, R, Rest), !.
parseExpr([-|T], [-|R], Rest):- parseExpr(T, R, Rest), !.
parseExpr([<|T], [<|R], Rest):- parseExpr(T, R, Rest), !.
parseExpr([==|T], [==|R], Rest):- parseExpr(T, R, Rest), !.
parseExpr([H|T], [V|R], Rest):- parseExpr(T, R, Rest), atom_number(H, V), !.
parseExpr([H|T], [H|R], Rest):- parseExpr(T, R, Rest).


/* Predicat folosit pentru delimitarea unui bloc de program */
%parseProg(L, R, Rest, Cnt)
parseProg(['}'|T], [], T, 1):- !.
parseProg(['}'|T], ['}'|R], Rest, Cnt):- C is Cnt - 1, parseProg(T, R, Rest, C), !.
parseProg(['{'|T], R, Rest, 0):- parseProg(T, R, Rest, 1), !.
parseProg(['{'|T], ['{'|R], Rest, Cnt):- C is Cnt + 1, parseProg(T, R, Rest, C), !.
parseProg([H|T], [H|R], Rest, Cnt):- parseProg(T, R, Rest, Cnt).


/* Predicat folosit pentru detectarea valorii unei variabile sau a unui numar */
%value(Var, L, R)
value(Var, _, Var):- number(Var), !.
value(Var, [[Var, R]|_], R):- !.
value(Var, [_|T], R):- value(Var, T, R).


/* Predicat folosit pentru evaluarea unei expresii folosind forma postfixata */
%evalExpr(L, S1, S2, P, Vars, R)
evalExpr([], [], [], L, Vars, R):- 	reverse(L, [H1, H2|P]),
									evalExpr([], [], [H2,H1], P, Vars, R), !.
evalExpr([], [], [H], [], Vars, R):- value(H, Vars, R), !.
evalExpr([], [], [H1,H2|T], [*|P], Vars, R):- 	value(H1, Vars, A),
												value(H2, Vars, B),
												C is B * A,
												evalExpr([], [], [C|T], P, Vars, R), !.
evalExpr([], [], [H1,H2|T], [+|P], Vars, R):- 	value(H1, Vars, A),
												value(H2, Vars, B),
												C is B + A,
												evalExpr([], [], [C|T], P, Vars, R), !.
evalExpr([], [], [H1,H2|T], [-|P], Vars, R):- 	value(H1, Vars, A),
												value(H2, Vars, B),
												C is B - A,
												evalExpr([], [], [C|T], P, Vars, R), !.
evalExpr([], [], [H1,H2|T], [<|P], Vars, R):- 	value(H1, Vars, A),
												value(H2, Vars, B),
												B < A,
												evalExpr([], [], [1|T], P, Vars, R), !.
evalExpr([], [], [H1,H2|T], [<|P], Vars, R):- 	value(H1, Vars, A),
												value(H2, Vars, B),
												not(B < A),
												evalExpr([], [], [0|T], P, Vars, R), !.
evalExpr([], [], [H1,H2|T], [==|P], Vars, R):- 	value(H1, Vars, A),
												value(H2, Vars, B),
												B =:= A,
												evalExpr([], [], [1|T], P, Vars, R), !.
evalExpr([], [], [H1,H2|T], [==|P], Vars, R):- 	value(H1, Vars, A),
												value(H2, Vars, B),
												not(B =:= A),
												evalExpr([], [], [0|T], P, Vars, R), !.
evalExpr([], [], T, [H|P], Vars, R):- evalExpr([], [], [H|T], P, Vars, R), !.

evalExpr([], [H|S], [], P, Vars, R):- evalExpr([], S, [], [H|P], Vars, R), !.
evalExpr([*|L], [], [], P, Vars, R):- evalExpr(L, [*], [], P, Vars, R), !.
evalExpr([+|L], [], [], P, Vars, R):- evalExpr(L, [+], [], P, Vars, R), !.
evalExpr([-|L], [], [], P, Vars, R):- evalExpr(L, [-], [], P, Vars, R), !.
evalExpr([<|L], [], [], P, Vars, R):- evalExpr(L, [<], [], P, Vars, R), !.
evalExpr([==|L], [], [], P, Vars, R):- evalExpr(L, [==], [], P, Vars, R), !.
evalExpr([*|L], [*|S], [], P, Vars, R):- evalExpr([*|L], S, [], [*|P], Vars, R), !.
evalExpr([*|L], S, [], P, Vars, R):- evalExpr(L, [*|S], [], P, Vars, R), !.
evalExpr([+|L], [*|S], [], P, Vars, R):- evalExpr([+|L], S, [], [*|P], Vars, R), !.
evalExpr([+|L], [+|S], [], P, Vars, R):- evalExpr([+|L], S, [], [+|P], Vars, R), !.
evalExpr([+|L], [-|S], [], P, Vars, R):- evalExpr([+|L], S, [], [-|P], Vars, R), !.
evalExpr([+|L], S, [], P, Vars, R):- evalExpr(L, [+|S], [], P, Vars, R), !.
evalExpr([-|L], [*|S], [], P, Vars, R):- evalExpr([-|L], S, [], [*|P], Vars, R), !.
evalExpr([-|L], [+|S], [], P, Vars, R):- evalExpr([-|L], S, [], [+|P], Vars, R), !.
evalExpr([-|L], [-|S], [], P, Vars, R):- evalExpr([-|L], S, [], [-|P], Vars, R), !.
evalExpr([-|L], S, [], P, Vars, R):- evalExpr(L, [-|S], [], P, Vars, R), !.
evalExpr([<|L], [*|S], [], P, Vars, R):- evalExpr([<|L], S, [], [*|P], Vars, R), !.
evalExpr([<|L], [+|S], [], P, Vars, R):- evalExpr([<|L], S, [], [+|P], Vars, R), !.
evalExpr([<|L], [-|S], [], P, Vars, R):- evalExpr([<|L], S, [], [-|P], Vars, R), !.
evalExpr([<|L], [<|S], [], P, Vars, R):- evalExpr([<|L], S, [], [<|P], Vars, R), !.
evalExpr([<|L], S, [], P, Vars, R):- evalExpr(L, [<|S], [], P, Vars, R), !.
evalExpr([==|L], [*|S], [], P, Vars, R):- evalExpr([==|L], S, [], [*|P], Vars, R), !.
evalExpr([==|L], [+|S], [], P, Vars, R):- evalExpr([==|L], S, [], [+|P], Vars, R), !.
evalExpr([==|L], [-|S], [], P, Vars, R):- evalExpr([==|L], S, [], [-|P], Vars, R), !.
evalExpr([==|L], [<|S], [], P, Vars, R):- evalExpr([==|L], S, [], [<|P], Vars, R), !.
evalExpr([==|L], [==|S], [], P, Vars, R):- evalExpr([==|L], S, [], [==|P], Vars, R), !.
evalExpr([==|L], S, [], P, Vars, R):- evalExpr(L, [==|S], [], P, Vars, R), !.
evalExpr([H|L], S, [], P, Vars, R):- evalExpr(L, S, [], [H|P], Vars, R), !.


/* Predicat folosit pentru executarea unui program folosind un vector de variabile si valorile lor */
%parseVars(L, Vars, VarAux, R)
/*Program fara return*/
parseVars([], Vars, Vars, 'x'):- !.

/* Program ; */
parseVars([;,'}'|T], Vars, VarAux, R):- parseVars(T, Vars, VarAux, R), !.
parseVars([;|T], Vars, VarAux, R):- parseVars(T, Vars, VarAux, R), !.

%Return program
parseVars([return|T], Vars, _, R):- parseExpr(T, P, _), evalExpr(P, [], [], [], Vars, R), !.
parseVars([return|_], _, _, 'e').

/* Program If */
parseVars([if, '('|T], Vars, VarAux, R):- 	parseExpr(T, Expr, [then, '{'|Rest1]),
											evalExpr(Expr, [], [], [], Vars, 1),
											parseProg(['{'|Rest1], Prog, [else, '{'|_], 0),
											parseVars(Prog, Vars, VarAux, R),
											number(R), !.
parseVars([if, '('|T], Vars, VarAux, R):- 	parseExpr(T, Expr, [then ,'{'|Rest1]),
											evalExpr(Expr, [], [], [], Vars, 0),
											parseProg(['{'|Rest1], _, [else, '{'|Rest2], 0),
											parseProg(['{'|Rest2], Prog, _, 0),
											parseVars(Prog, Vars, VarAux, R),
											number(R), !.
parseVars([if, '('|T], Vars, VarAux, R):- 	parseExpr(T, Expr, [then, '{'|Rest1]),
											evalExpr(Expr, [], [], [], Vars, 1),
											parseProg(['{'|Rest1], Prog, [else, '{'|Rest2], 0),
											parseVars(Prog, Vars, Vars2, 'x'),
											parseProg(['{'|Rest2], _, Rest3, 0),
											parseVars(Rest3, Vars2, VarAux, R), !.
parseVars([if, '('|T], Vars, VarAux, R):- 	parseExpr(T, Expr, [then, '{'|Rest1]),
											evalExpr(Expr, [], [], [], Vars, 0),
											parseProg(['{'|Rest1], _, [else, '{'|Rest2], 0),
											parseProg(['{'|Rest2], Prog, Rest3, 0),
											parseVars(Prog, Vars, Vars2, 'x'),
											parseVars(Rest3, Vars2, VarAux, R), !.
parseVars([if|_], _, _, 'e').

/* Program While */
parseVars([while, '('|T], Vars, VarAux, R):-	parseExpr(T, Expr, ['{'|Rest1]),
												evalExpr(Expr, [], [], [], Vars, 1),
												parseProg(['{'|Rest1], Prog, _, 0),
												parseVars(Prog, Vars, VarAux, R),
												number(R), !.
parseVars([while, '('|T], Vars, VarAux, R):-	parseExpr(T, Expr, ['{'|Rest1]),
												evalExpr(Expr, [], [], [], Vars, 1),
												parseProg(['{'|Rest1], Prog, _, 0),
												parseVars(Prog, Vars, Vars2, 'x'),
												parseVars([while, '('|T], Vars2, VarAux, R), !.
parseVars([while, '('|T], Vars, VarAux, R):-	parseExpr(T, Expr, ['{'|Rest1]),
												evalExpr(Expr, [], [], [], Vars, 0),
												parseProg(['{'|Rest1], _, Rest2, 0),
												parseVars(Rest2, Vars, VarAux, R), !.
parseVars([while|_], _, _, 'e').

/* Program de atribuire */
%parseVars([_, =, ;|_], _, _, 'e'):- !.
parseVars([H, =|T], Vars, VarAux, R):- 	not(atom_number(H, _)),
										parseExpr(T, L, Rest),
										evalExpr(L, [], [], [], Vars, P),
										parseVars(Rest, [[H, P]|Vars], VarAux, R), !.
parseVars([_, =|_], _, _, 'e').


parseInputAux(L,R):- parseVars(L, [], _, R).

parseInput(F,R):- read_file_to_string(F,S,[]), lparse(S,L), parseInputAux(L, R), !.
