package decorator;

import homeworkPP.Context;
import visitor.Visitor;

// Constant or variable type of expression
public class Value extends Elem<Integer> {
	private String s;

	public String getS() {
		return s;
	}

	public Value() {
		s = "";
	}

	public Value(String s) {
		this.s = s;
	}

	// Evaluate the string
	@Override
	public Integer eval(Context c) {
		// If it is a number, it returns that number
		if ('0' <= s.charAt(0) && s.charAt(0) <= '9')
			return Integer.parseInt(s);
		// Otherwise, return the value of the variable
		return c.valueOf(s);
	}

	// Accept function for Visitor design pattern
	public void accept(Visitor v, Context c) {
		v.visit(this, c);
	}
}