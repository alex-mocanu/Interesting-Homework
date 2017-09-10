package decorator;

import homeworkPP.Context;
import homeworkPP.Main;
import visitor.Visitor;

// Multiplication expression
public class MultExpr extends Elem<Integer> {
	private String s;

	public String getS() {
		return s;
	}

	public MultExpr() {
		s = "";
	}

	public MultExpr(String s) {
		this.s = s;
	}

	@Override
	public Integer eval(Context c) {
		// Extract the values to be multiplied
		String expr = s.substring(3, s.length() - 1);
		String[] list = Main.splitList(expr);
		// Evaluate the operands
		Integer val1 = Main.evalExpression(list[0], c);
		Integer val2 = Main.evalExpression(list[1], c);
		// Return the product of the operands
		return val1 * val2;
	}

	// Accept function for Visitor design pattern
	public void accept(Visitor v, Context c) {
		v.visit(this, c);
		// Extract the operands
		String expr = s.substring(3, s.length() - 1);
		String[] list = Main.splitList(expr);
		// Apply accept to each operand
		Elem.type1(list[0]).accept(v, c);
		Elem.type1(list[1]).accept(v, c);
	}
}