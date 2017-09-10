package decorator;

import homeworkPP.Context;
import homeworkPP.Main;
import visitor.Visitor;

// Equality check expression
public class EqualExpr extends Elem<Boolean> {
	private String s;

	public String getS() {
		return s;
	}

	public EqualExpr() {
		s = "";
	}

	public EqualExpr(String s) {
		this.s = s;
	}

	@Override
	public Boolean eval(Context c) {
		// Extract the values to be compared
		String expr = s.substring(4, s.length() - 1);
		String[] list = Main.splitList(expr);
		// Evaluate each member
		Integer val1 = Main.evalExpression(list[0], c);
		Integer val2 = Main.evalExpression(list[1], c);
		// Return the result of comparing the members
		return val1 == val2;
	}

	// Accept function for Visitor design pattern
	public void accept(Visitor v, Context c) {
		v.visit(this, c);
		// Extract the members
		String expr = s.substring(4, s.length() - 1);
		String[] list = Main.splitList(expr);
		// Apply accept to each member
		Elem.type1(list[0]).accept(v, c);
		Elem.type1(list[1]).accept(v, c);
	}
}