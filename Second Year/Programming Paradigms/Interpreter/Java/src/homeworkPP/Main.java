package homeworkPP;

import java.util.ArrayList;
import java.util.LinkedList;
import java.util.List;

import decorator.*;
import visitor.CheckingVisitor;

public class Main {
	// Open parenthesis - discovered closed parenthesis
	public static Integer paranthesis = 0;
	// Boolean value for correctness of the program
	public static Boolean correctProgram = true;

	/**
	 * IMPORTANT! Your solution will have to implement this method.
	 * 
	 * @param exp
	 *            - a string, which represents an expression (that follows the
	 *            specification in the homework);
	 * @param c
	 *            - the context (a one-to-one association between variables and
	 *            values);
	 * @return - the result of the evaluation of the expression;
	 */
	public static Integer evalExpression(String exp, Context c) {
		if (exp.charAt(0) != '[') {
			Value val = new Value(exp.trim());
			return val.eval(c);
		}

		switch (exp.charAt(1)) {
		case '+': {
			AddExpr expr = new AddExpr(exp.trim());
			return expr.eval(c);
		}
		default: {
			MultExpr expr = new MultExpr(exp.trim());
			return expr.eval(c);
		}
		}
	}

	/**
	 * IMPORTANT! Your solution will have to implement this method.
	 * 
	 * @param program
	 *            - a string, which represents a program (that follows the
	 *            specification in the homework);
	 * @return - the result of the evaluation of the expression;
	 */
	public static Integer evalProgram(String program) {
		Context c = new Context();
		Prog p = Prog.type(program.trim());
		p.eval(c);
		return c.valueOf("return");
	}

	/**
	 * IMPORTANT! Your solution will have to implement this method.
	 * 
	 * @param program
	 *            - a string, which represents a program (that follows the
	 *            specification in the homework);
	 * @return - whether the given program follow the syntax rules specified in
	 *         the homework (always return a value and always use variables that
	 *         are "in scope");
	 */

	public static Boolean checkCorrectness(String program) {
		correctProgram = true;
		Context c = new Context();
		program = program.trim();
		// Start by counting the open parenthesis
		for (int i = 0; i < program.length(); i++)
			if (program.charAt(i) == '[')
				paranthesis++;
		// Check the program
		Prog.type(program).accept(new CheckingVisitor(), c, new ArrayList<String>());

		// If there is no return statement, the program is wrong
		try {
			c.valueOf("return");
		} catch (RuntimeException e) {
			return false;
		}
		return correctProgram;
	}

	/**
	 *
	 * @param s
	 *            - a string, that contains a list of programs, each program
	 *            starting with a '[' and ending with a matching ']'. Programs
	 *            are separated by the whitespace character;
	 * @return - array of strings, each element in the array representing a
	 *         program; Example: "[* [+ 1 2] 3] [* 4 5]" -> "[* [+ 1 2] 3]" &
	 *         "[* 4 5]";
	 */
	public static String[] splitList(String s) {
		List<String> l = new LinkedList<String>();
		int inside = 0;
		int start = 0, stop = 0;
		for (int i = 0; i < s.length(); i++) {
			if (s.charAt(i) == '[') {
				inside++;
				stop++;
				continue;
			}
			if (s.charAt(i) == ']') {
				inside--;
				stop++;
				continue;
			}
			if (s.charAt(i) == ' ' && inside == 0) {
				l.add(s.substring(start, stop));
				start = i + 1; // starting after whitespace
				stop = start;

				continue;
			}
			stop++; // no special case encountered
		}
		if (stop > start) {
			l.add(s.substring(start, stop));
		}

		return l.toArray(new String[l.size()]);
	}

	public static void main(String[] args) {
		/* Suggestion: use it for testing */
		String cp2 = "[; [; [= x 10] [if [< x 5] [= x 2] [if [< x 20] [= x 126] [= x 4]]]] [return x]]";
		System.out.println(checkCorrectness(cp2));
		System.out.println(evalProgram(cp2));
	}
}
