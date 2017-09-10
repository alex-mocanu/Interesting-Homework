package decorator;

import homeworkPP.Context;
import homeworkPP.Main;
import visitor.Visitor;

// Comparison expression
public class CompExpr extends Elem<Boolean> {
	private String s;

	public String getS() {
		return s;
	}

	public CompExpr() {
		s = "";
	}

	public CompExpr(String s) {
		this.s = s;
	}

	@Override
	public Boolean eval(Context c) {
		// Extract the members of the inequality
		String expr = s.substring(3, s.length() - 1);
		String[] list = Main.splitList(expr);
		// Evaluate the members
		Integer val1 = Main.evalExpression(list[0], c);
		Integer val2 = Main.evalExpression(list[1], c);
		// Return the result of the comparison
		return val1 < val2;
	}

	// Accept function for Visitor design pattern
	public void accept(Visitor v, Context c) {
		v.visit(this, c);
		// Extract the members
		String expr = s.substring(3, s.length() - 1);
		String[] list = Main.splitList(expr);
		// Apply accept to each member
		Elem.type1(list[0]).accept(v, c);
		Elem.type1(list[1]).accept(v, c);
	}
}