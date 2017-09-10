package decorator;

import homeworkPP.Context;
import visitor.Visitor;

// Abstract class for expressions
public abstract class Elem<T> {
	public abstract T eval(Context c);

	/**
	 * Accept function for Visitor design pattern
	 * 
	 * @param v
	 *            - a visitor object
	 * @param c
	 *            - the current context
	 */
	public abstract void accept(Visitor v, Context c);

	// Function for deciding the type of expression that returns an integer
	public static Elem<Integer> type1(String s) {
		if (s.charAt(0) != '[')
			return new Value(s);
		else if (s.charAt(1) == '+')
			return new AddExpr(s);
		else
			return new MultExpr(s);
	}

	// Function for deciding the type of expression that returns a boolean
	public static Elem<Boolean> type2(String s) {
		if (s.charAt(1) == '=')
			return new EqualExpr(s);
		else
			return new CompExpr(s);
	}
}